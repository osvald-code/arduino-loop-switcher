
//changing this to true allows you to set pedal order with the debug if in the loop  
bool const DEBUG_MODE = true;

enum analog_pins {
    PEDAL_1 = A2,
    PEDAL_2 = A3,
    PEDAL_3 = A4
};

//consts for digital pins
enum digital_pins {
    TRANS_BYPASS = 2,
    TRANS_PEDALS = 3,
    RELAY_B1_P1 = 4,
    RELAY_B1_P2 = 5,
    RELAY_B2_P2 = 6,
    RELAY_B2_P3 = 7,
    RELAY_B3_P1 = 8,
    RELAY_B3_P3 = 9,
    RELAY_B4_P1 = 10,
    RELAY_B4_P2 = 11
};

//switch has a value of on or off 
struct Switch {
    bool isOn = false;
};

struct InputSwitch :Switch {
    int pin = -1;
};


struct OutputSwitch :Switch {
    digital_pins pin;
};

//container for pedal switches to retain order
struct PedalArray
{
    int selectedPedals[3] = { 0,0,0 };
    int size = 0;

    //adds pedal to next available position to list 
    void add(int pedalNum) {
        selectedPedals[size] = pedalNum;
        ++size;
    }

    //removes pedal number from list 
    void remove(int pedalNum)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (selectedPedals[i] == pedalNum)
            {
                if (i != size - 1) {
                    removal_sort(i);
                }
                selectedPedals[i] = 0;
                --size;
                return;
            }
        }
    }
    void removal_sort(int idx)
    {
        for (int i = idx; i < size - 1; ++i)
        {
            selectedPedals[i] = selectedPedals[i + 1];
        }
    }


};



//holds ON/OFF values for relays
struct InputRelay
{
    //amount of relays in use
    static int const SIZE = 8;sw

    //array of switches for relay 
    OutputSwitch outputSwitches[SIZE];

    //interator points to the switches about to be used
    int relayIterator = 0;

    //sets switches pin to related output pin
    InputRelay()
    {
        outputSwitches[0].pin = RELAY_B1_P1;
        outputSwitches[1].pin = RELAY_B1_P2;
        outputSwitches[2].pin = RELAY_B2_P2;
        outputSwitches[3].pin = RELAY_B2_P3;
        outputSwitches[4].pin = RELAY_B3_P1;
        outputSwitches[5].pin = RELAY_B3_P3;
        outputSwitches[6].pin = RELAY_B4_P1;
        outputSwitches[7].pin = RELAY_B4_P2;
    }

    //sets all switches to the off position
    void reset() {
        for (int i = 0; i < 8; ++i)
        {
            outputSwitches[i].isOn = false;
        }
    }

    //set switch at index to on position
    void setOn(int idx)
    {
        outputSwitches[idx].isOn = true;
    }

    void toggleAllSwitches()
    {
        for (int i = 0; i < SIZE; ++i)
        {
            //the relay is default ON, so the power has to send on to turn off..
            if (outputSwitches[i].isOn) {
                digitalWrite(outputSwitches[i].pin, LOW);
            }
            else {
                digitalWrite(outputSwitches[i].pin, HIGH);
            }
        };
    }
};

//InputSwitch decicated for pedals 
struct PedalSwitch :InputSwitch {
    int pedalNumber = 0;
}p1Switch, p2Switch, p3Switch;


void pedalSwitchState(PedalArray& pedals, PedalSwitch& iSwitch);

void setRelayValues(InputRelay& ir, int& pedalNum);

PedalArray pedalArray;
InputRelay inputRelay;
//holds the count of the engaged pedals 
int lastPedalSwitchCount = 0;

void setup() {

    //set output pins
    pinMode(RELAY_B1_P1, OUTPUT);
    pinMode(RELAY_B1_P2, OUTPUT);
    pinMode(RELAY_B2_P2, OUTPUT);
    pinMode(RELAY_B2_P3, OUTPUT);
    pinMode(RELAY_B3_P1, OUTPUT);
    pinMode(RELAY_B3_P3, OUTPUT);
    pinMode(RELAY_B4_P1, OUTPUT);
    pinMode(RELAY_B4_P2, OUTPUT);
    pinMode(TRANS_BYPASS, OUTPUT);
    pinMode(TRANS_PEDALS, OUTPUT);

    // set switch pins
    p1Switch.pin = PEDAL_1;
    p2Switch.pin = PEDAL_2;
    p3Switch.pin = PEDAL_3;

    //set pedal number 
    p1Switch.pedalNumber = 1;
    p2Switch.pedalNumber = 2;
    p3Switch.pedalNumber = 3;

    lastPedalSwitchCount = pedalArray.size;
    //digitalWrite(RELAY_B1_P1, HIGH);
    //digitalWrite(RELAY_B1_P2, HIGH);
    //digitalWrite(RELAY_B2_P2, HIGH);
    //digitalWrite(RELAY_B2_P3, HIGH);
    //digitalWrite(RELAY_B3_P1, HIGH);
    //digitalWrite(RELAY_B3_P3, HIGH);
    //digitalWrite(RELAY_B4_P1, HIGH);
    //digitalWrite(RELAY_B4_P2, HIGH);
}

void loop() {

    //if there are pedals selected 
    if (pedalArray.size > 0) {
       //set transisters to not by pass 
       // digitalWrite(TRANS_BYPASS, LOW);
       //digitalWrite(TRANS_PEDALS, HIGH);

        //if there are pedal switches engaged/ if the amount of selected pedals has changed  
        if (lastPedalSwitchCount != pedalArray.size) {

            //set all relay values to off (in code only, not physically)        
            inputRelay.reset();

            //loop through pedal switches 
            for (int i = 0; i < pedalArray.size; ++i)
            {
                //set the values in the realy that need to be on
                setRelayValues(inputRelay, pedalArray.selectedPedals[i]);
            }
            //physically switch all relays into desired position
            inputRelay.toggleAllSwitches();

            //reset the lastPedalSwitch count 
            lastPedalSwitchCount = pedalArray.size;
        }

    }
    else {
        //set transister to bypass
        //digitalWrite(TRANS_BYPASS, HIGH);
       	//digitalWrite(TRANS_PEDALS, LOW);
    }


    //check and set pedal switch state and order 
    pedalSwitchState(pedalArray, p1Switch);
    pedalSwitchState(pedalArray, p2Switch);
    pedalSwitchState(pedalArray, p3Switch);


}


//takes the pedalarray and pedal switch and decides the pedal switch is on or not
void pedalSwitchState(PedalArray& pedals, PedalSwitch& iSwitch)
{
    //if the switch isn't on but it is recieving signal 
    //analog read here
    if ((analogRead(iSwitch.pin) > 1000) && !iSwitch.isOn)
    {
        //set switch to ON
        iSwitch.isOn = true;
        //add to pedal array
        pedals.add(iSwitch.pedalNumber);

    }
    //else if pedal is not recieving signal but is on 
    else if ((iSwitch.isOn) && (analogRead(iSwitch.pin) < 1000))
    {
        //set switch to off 
        iSwitch.isOn = false;
        //remove pedal from order
        pedals.remove(iSwitch.pedalNumber);
    }
}



void setRelayValues(InputRelay& ir, int& pedalNum) {

    //first pedals block
    if (ir.relayIterator < 2)
    {
        if (pedalNum != 3) {
            ir.setOn(pedalNum - 1);
        }

    }
    //determines if activating first relay or second
    else if (ir.relayIterator % 2 == 0)
    {
        ir.setOn(ir.relayIterator);
    }
    else {
        ir.setOn(ir.relayIterator + 1);
    }
    //move the block
    ir.relayIterator = (pedalNum * 2);
}

