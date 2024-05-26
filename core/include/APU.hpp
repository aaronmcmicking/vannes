#pragma once

class APU{
    public:
        // see https://www.nesdev.org/wiki/APU#Registers 
        // UNUSED ADDRESSES: 0x4009, 0x400D, 0x4014
        enum APU_regs{
                SQ1_VOL    = 0x4000, 	 // ddlcvvvv 	Square wave 1, duty and volume
                SQ1_SWEEP  = 0x4001, 	 // epppnsss 	Square wave 1, sweep
                SQ1_LO 	   = 0x4002, 	 // pppppppp 	Square wave 1, period (LSB)
                SQ1_HI 	   = 0x4003, 	 // xxxxxppp 	Square wave 1, period (MSB) and counter load
                SQ2_VOL    = 0x4004, 	 // dd..vvvv 	Square wave 2, duty and volume
                SQ2_SWEEP  = 0x4005, 	 // epppnsss 	Square wave 2, sweep
                SQ2_LO 	   = 0x4006, 	 // pppppppp 	Square wave 2, period (LSB)
                SQ2_HI 	   = 0x4007, 	 // xxxxxppp 	Square wave 2, period (MSB) and counter load
                TRI_LINEAR = 0x4008, 	 // crrrrrrr 	Triangle wave, control and counter load
                TRI_LO 	   = 0x400A, 	 // pppppppp 	Triangle wave, period (LSB)
                TRI_HI 	   = 0x400B, 	 // xxxxxppp 	Triangle wave, period (MSB) and counter load
                NOISE_VOL  = 0x400C, 	 // ..lcvvvv 	Noise generator, flags and volume
                NOISE_CTRL = 0x400E, 	 // t...pppp 	Noise generator, tone and period
                NOISE_LEN  = 0x400F, 	 // lllll... 	Noise generator, counter load
                DMC_FREQ   = 0x4010, 	 // il..rrrr 	DMC: IRQ, flags, and rate
                DMC_RAW    = 0x4011, 	 // .xxxxxxx 	DMC: direct load
                DMC_START  = 0x4012, 	 // aaaaaaaa 	DMC, waveform start address
                DMC_LEN    = 0x4013, 	 // llllllll 	DMC, waveform length
                SND_CHN    = 0x4015, 	 // ...dnt21 	Sound channel enable
                JOY1 	   = 0x4016, 	 // ...xxxxd 	Joystick 1 (read)
                JOY2 	   = 0x4017, 	 // mi...... 	Frame counter mode and IRQ
        };

    private:
};
