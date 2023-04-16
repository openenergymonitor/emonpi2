// A simplified electricity monitor example
// - 1 Voltage and 1 Current channel
// - ADC Mode: Continuous Conversion 
// - accumSwap based on EmonLibDB implementation
// - no phase calibration in this example

#define Serial Serial3

static byte ADC_Sequence[2] = {0,3}; 
unsigned int ADC_Counts = 1 << 12;

// Structure to hold accumulation variables
struct Accum
{
  uint32_t sampleSets;
  uint64_t sum_Vsquared;
  uint64_t sum_Isquared;
  int64_t sumP;
  int64_t cumV_deltas;
  int64_t cumI_deltas;
};

// A fast way too swap between two sets of data structures
void accumSwap(volatile Accum **p_c, volatile Accum **p_p ) 
{
  volatile Accum *tmp;
  tmp = *p_p;
  *p_p = *p_c;
  *p_c = tmp;
}

// Two sets of accumulation variables
volatile struct Accum accum_A;
volatile struct Accum accum_B;
volatile struct Accum *coll = &accum_A;
volatile struct Accum *proc = &accum_B;

// Flag for when readings are ready to print
volatile bool reading_ready = false;

// Calibration coefficients
const double vcal = 809.0 * 1.024 / 4096;
const double ical = 60.06 * 1.024 / 4096;

// ADC multiplexer position
int muxpos = 0;
int next = 0;

// Calculate sample period:
// Sample Period = (Sample cycles + SAMPCTRL + Conversion cycles) / (Clock / Prescaler)
// page 498 of the datasheet
// SAMPCTRL = 14
// Clock = 24 MHz
// Prescaler = 32
// DIV 32: (2+14+13.5)÷(24÷32) = 39.3 us
// DIV 48: (2+14+13.5)÷(24÷48) = 59.0 us
const double samplePeriod = (2+14+13.5)/(24.0/32.0);
// 50 cycles per second, 20,000 us per cycle
const unsigned int sampleSetsToCapture = (int)((double)50*20000/(samplePeriod*2));

void setup() {
  Serial.begin(115200);
  Serial.println("AVR-DB Simplified Electricity Monitor Example");
  
  // Digital output to monitor ISR timing (CT7)
  PORTE.DIR = PIN2_bm;

  // Setup ADC in continuous conversion mode
  ADC0.SAMPCTRL = 14;
  ADC0.CTRLD |= 0x0;
  
  // Set ADC reference to 1.024V
  VREF.ADC0REF = VREF_REFSEL_1V024_gc;

  // Set ADC clock to 24 MHz / 32 = 750 kHz
  ADC0.CTRLC = ADC_PRESC_DIV32_gc;

  // Enable ADC
  ADC0.CTRLA = ADC_ENABLE_bm;

  // Set ADC resolution to 12 bits
  ADC0.CTRLA |= ADC_RESSEL_12BIT_gc;

  // Set ADC to free running mode
  ADC0.CTRLA |= ADC_FREERUN_bm;

  // Set ADC input to ADC0
  ADC0.MUXPOS = 0;

  // Enable ADC interrupt
  ADC0.INTCTRL |= ADC_RESRDY_bm;

  // Start first conversion
  ADC0.COMMAND = ADC_STCONV_bm;

  // Enable global interrupts
  sei();
}

void loop() {
  // Print readings when ready
  if (reading_ready) {
    reading_ready = false;

    // Calculate RMS values, active power, apparent power and power factor
    uint32_t n = proc->sampleSets;
    double Vrms = sqrt(((double)proc->sum_Vsquared/n)-((double)proc->cumV_deltas*proc->cumV_deltas/n/n)) * vcal;
    double Irms = sqrt(((double)proc->sum_Isquared/n)-((double)proc->cumI_deltas*proc->cumI_deltas/n/n)) * ical;
    double activePower = (proc->sumP/n - (double)proc->cumV_deltas * proc->cumI_deltas/n/n) * vcal * ical;    
    double apparentPower = Vrms * Irms;
    double powerFactor = activePower / apparentPower;
    
    // Reset process accumulation variables
    memset(proc, 0, sizeof(Accum));
    
    // Print readings
    Serial.print(Vrms);
    Serial.print(' ');
    Serial.print(Irms);
    Serial.print(' ');
    Serial.print(activePower);
    Serial.print(' ');
    Serial.print(apparentPower);
    Serial.print(' ');
    Serial.println(powerFactor,3);
    delay(50);
  }
}

// ADC interrupt
ISR(ADC0_RESRDY_vect) 
{
  // Clear interrupt flag
  ADC0.INTFLAGS = ADC_RESRDY_bm;
  // Toggle CT7 to monitor ISR timing
  PORTE.OUT |= PIN2_bm;

  static int16_t voltage,current;
  int16_t volatile rawSample;

  // Read ADC result
  rawSample = ADC0.RES;

  // Set next ADC channel
  next = muxpos + 2;
  if (next>1) next -= 2;
  
  // Set ADC channel
  ADC0.MUXPOS = ADC_Sequence[next];

  // muxpos = 0: Voltage
  if (muxpos==0) {
    // initial crude bias offset removal
    voltage = rawSample - (ADC_Counts >> 1);   
    // accumulate offset for later removal 
    coll->cumV_deltas += voltage;
    // accumulate squared voltage for later RMS calculation
    coll->sum_Vsquared += ((long)voltage * voltage);
    
  // muxpos = 1: Current
  } else if (muxpos==1) {
    // initial crude bias offset removal
    current = rawSample - (ADC_Counts >> 1);
    // accumulate offset for later removal
    coll->cumI_deltas += current;
    // accumulate squared current for later RMS calculation
    coll->sum_Isquared += ((long)current * current);
    // accumulate power for later active power calculation
    coll->sumP += ((long)voltage * current);
    // increment sample set counter
    coll->sampleSets++;

    // If we have enough samples, swap accumulation variables
    if (coll->sampleSets>sampleSetsToCapture) {
      accumSwap(&coll, &proc);
      // Set flag to print readings
      reading_ready = true;
    }
  }
  
  // advance to next channel
  muxpos++;
  if (muxpos>1) muxpos = 0;

  // Toggle CT7 to monitor ISR timing
  PORTE.OUT &=~PIN2_bm;
}
