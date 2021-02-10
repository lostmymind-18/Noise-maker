#include<iostream>
#include<Windows.h>
using namespace std;
#include "olcNoiseMaker.h"
#define Sine 0
#define Square 1
#define Triangle 2
#define SawAna 3
#define SawDig 4

double dFrequencyOutput = 0.0;
double w(double dHezt)
{
    return dHezt * 2.0 * PI;
}

double osc(double dHertz, double dTime, int nType, double dLFOHertz = 0.0, double dLFOAmplitude = 0.0)
{
    double dFreq = w(dHertz) * dTime + dLFOAmplitude * dHertz * sin(w(dLFOHertz) * dTime);
    switch (nType)
    {
    case Sine: //Sine wave
        return sin(dFreq);

    case Square: //Square wave
        return sin(dFreq) > 0 ? 1 : -1;

    case Triangle://Triangle wave
        return asin((dFreq) * 2.0 / PI);
    case SawAna://Saw wave(analog/warm/slow)
    {
        double dOutPut = 0.0;
        for (double i = 1.0; i < 50.0; i++)
        {
            dOutPut += (sin(i * dFreq)) / i;
        }
        return dOutPut * (2.0 / PI);
    }
    case SawDig://Saw wave(optimised/harsh/fast)
        return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));
    default: return 0.0;
    }
}
double hold = 0.0;
struct sEnvelopeADSK {
    double dAttackTime;
    double dDecayTime;
    double dReleaseTime;
    double dSustainAmplitude;
    double dStartAmplitude;
    double dTriggerOnTime;
    double dTriggerOffTime;
    bool bNoteOn;
    double getAmplitute(double dTime)
    {
        double dAmplitude = 0.0;
        double dLifeTime = dTime - dTriggerOnTime;

        if (bNoteOn)
        {
            //Attack, Decay, Sustain
            //Attack
            if (dLifeTime <= dAttackTime)
            {
                dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;
            }
            //Decay
            else if (dLifeTime <= (dAttackTime + dDecayTime))
                dAmplitude = dStartAmplitude - (dStartAmplitude - dSustainAmplitude) / dDecayTime * (dLifeTime - dAttackTime);
            else

                dAmplitude = dSustainAmplitude;
            hold = dAmplitude;
        }
        if (!bNoteOn)
        {
            if ((dTime - dTriggerOffTime) > 0 && (dTime - dTriggerOffTime) <= dReleaseTime)
            {
                dAmplitude = hold - dSustainAmplitude / dReleaseTime * (dTime - dTriggerOffTime);
            }
            if (dAmplitude < 0.0001) dAmplitude = 0.0;
        }

        return dAmplitude;
    }

    void NoteOn(double dTimeOn)
    {
        dTriggerOnTime = dTimeOn;
        bNoteOn = true;
    }
    void NoteOff(double dTimeOff)
    {
        dTriggerOffTime = dTimeOff;
        bNoteOn = false;
    }
};

struct Instrument {
    sEnvelopeADSK env;
    double dVolum;
    virtual double sound(double dTime) = 0;
};
struct bell :public Instrument {
    bell()
    {
        env.dAttackTime = 0.01;
        env.dDecayTime = 0.01;
        env.dReleaseTime = 0.05;
        env.dStartAmplitude = 1.0;
        env.dSustainAmplitude = 0.5;

    }
    double sound(double dTime)
    {
        double dOutput = env.getAmplitute(dTime) *
            (
                1.0 * osc(dFrequencyOutput * 2.0, dTime, Sine, 5.0)
                + 0.5 * osc(dFrequencyOutput * 3.0, dTime, Sine)
                + 0.25 * osc(dFrequencyOutput * 4.0, dTime, Sine)
                );
        return dOutput;
    }
};

Instrument* voice = NULL;//So it can point to any inherit object!


double MakeNoise(double dTime)
{
    double dOutput = voice->sound(dTime);
    return dOutput;
}

int main()
{
    wcout << endl <<
        "......................................................................" << endl <<
        "         ..  ........   ........   ........  ......      ............" << endl <<
        "         ..  .......  .  .......  .  ......  ....  .......  ..........." << endl <<
        "        ...  ......  ...  ......  ..  .....  ...  .........  ............" << endl <<
        "      .....  .....  .....  .....  ...  ....  ...  .........  ............" << endl <<
        " ..........  ....           ....  ....  ...  ...  .........  ..........." << endl <<
        " ..........  ...  .........  ...  .....  ..  ....  .......  ............" << endl <<
        " ..........  ..  ...........  ..  ......  .  .....       ............." << endl <<
        "......................................................................" << endl;

    //Get all sound hardware
    std::vector<wstring> devices = olcNoiseMaker<short>::Enumerate();

    //Display finding devices
    for (auto d : devices) wcout << "Found Output device: " << d << endl;

    //Create sound machine!!
    olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);
    voice = new bell();
    //Link noise function with sound machine
    sound.SetUserFunction(MakeNoise);
    double dOctiveBaseFrequency = 110.00; //A1
    double d12thRootOf2 = pow(2.0, 1.0 / 12.0);
    bool bKeyPressed = false;
    int cCurrentKey = -1;


    wcout << endl <<
        "|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |" << endl <<
        "|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |" << endl <<
        "|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << endl <<
        "|     |     |     |     |     |     |     |     |     |     |" << endl <<
        "|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << endl <<
        "|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << endl << endl;

    bool key = false;
    bool badSound;
    while (1)
    {
        
        // Add a keybroad like piano
        key = false;
        for (int i = 0; i < 26; i++)
        {
            if (GetAsyncKeyState((unsigned char)("QWERTYUIOPASDFGHJKLZXCVBNM"[i])) && 0x8000)
            {
                if (i != cCurrentKey)
                {
                    cCurrentKey = i;
                    voice->env.NoteOn(sound.GetTime());
                    dFrequencyOutput = dOctiveBaseFrequency * pow(d12thRootOf2, i);
                }
           
                key = true;
            }
        }
        if (!key)
        {
            if (cCurrentKey != -1)
            {
                cCurrentKey = -1;
                voice->env.NoteOff(sound.GetTime());
            }
        }
    }
    return 0;
}