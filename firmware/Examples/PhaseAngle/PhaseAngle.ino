// Phase angle measurement
// 
// Timing: 27.2us (i sample?) & 28.0us (v sample?)

#define Serial Serial3

static byte ADC_Sequence[2] = {0,3}; 
unsigned int ADC_Counts = 1 << 12;

struct Accum
{
  uint32_t sampleSets;
  uint64_t sum_Vsquared;
  uint64_t sum_Isquared;
  int64_t sumP;
  int64_t cumV_deltas;
  int64_t cumI_deltas;
  
  int64_t sum_sin_V;
  int64_t sum_cos_V;
  int64_t sum_sin_I;
  int64_t sum_cos_I;
};

void accumSwap(volatile Accum **p_c, volatile Accum **p_p ) 
{
  volatile Accum *tmp;
  tmp = *p_p;
  *p_p = *p_c;
  *p_c = tmp;
}



volatile struct Accum accum_A;
volatile struct Accum accum_B;
volatile struct Accum *coll = &accum_A;
volatile struct Accum *proc = &accum_B;

int sin_values[254];
int cos_values[254];

volatile bool reading_ready = false;

const double vcal = 809.0 * 1.024 / 4096;
const double ical = 60.06 * 1.024 / 4096;

const double twopi = 2 * 3.14159265;

int muxpos = 0;
int next = 0;

const double samplePeriod = (2+14+13.5)/(24.0/32.0);
const unsigned int sampleSetsToCapture = (int)((double)50*20000/(samplePeriod*2));
const unsigned int sampleSetsPerPeriod = (int)((double)20000/(samplePeriod*2))+0.5;
const double samplePhaseOffset = 360 * samplePeriod / 20000;

void setup() {
  Serial.begin(115200);
  Serial.println("AVR-DB Phase Angle");
  Serial.print("samplePeriod: ");
  Serial.println(samplePeriod);
  Serial.print("sampleSetsPerPeriod: ");
  Serial.println(sampleSetsPerPeriod);
  Serial.print("sampleSetsToCapture: ");
  Serial.println(sampleSetsToCapture);

  // Generate sin and cos tables for faster processing
  // Scaling of values by 1000 allows use of integer data types
  // and does not effect final result
  double time_rad = 0;
  double time_rad_inc = 2 * twopi * samplePeriod / 20000;
  for (int i=0; i<sampleSetsPerPeriod; i++) {
    time_rad = i*time_rad_inc;
    sin_values[i] = sin(time_rad)*1000;
    cos_values[i] = cos(time_rad)*1000;
  }

  // Digital output to monitor ISR timing
  PORTE.DIR = PIN2_bm;

  // Setup ADC in continuous conversion mode
  // DIV 48 = (2+14+13.5)÷(24÷48) = 59.0 us
  
  ADC0.SAMPCTRL = 14;
  ADC0.CTRLD |= 0x0;
  
  VREF.ADC0REF = VREF_REFSEL_1V024_gc;
  ADC0.CTRLC = ADC_PRESC_DIV32_gc;

  ADC0.CTRLA = ADC_ENABLE_bm;
  ADC0.CTRLA |= ADC_RESSEL_12BIT_gc;
  ADC0.CTRLA |= ADC_FREERUN_bm;
  ADC0.MUXPOS = 0;
  ADC0.INTCTRL |= ADC_RESRDY_bm;

  ADC0.COMMAND = ADC_STCONV_bm;

  sei();
}

void loop() {
  if (reading_ready) {
    reading_ready = false;

    uint32_t n = proc->sampleSets;
    double Vrms = sqrt(((double)proc->sum_Vsquared/n)-((double)proc->cumV_deltas*proc->cumV_deltas/n/n)) * vcal;
    double Irms = sqrt(((double)proc->sum_Isquared/n)-((double)proc->cumI_deltas*proc->cumI_deltas/n/n)) * ical;
    double activePower = (proc->sumP/n - (double)proc->cumV_deltas * proc->cumI_deltas/n/n) * vcal * ical;    
    double apparentPower = Vrms * Irms;
    double powerFactor = activePower / apparentPower;
    double power_factor_degrees = 360*(acos(powerFactor)/twopi);

    double angleV = 360*atan2(((double)proc->sum_cos_V/n),((double)proc->sum_sin_V/n))/twopi;
    double angleI = 360*atan2(((double)proc->sum_cos_I/n),((double)proc->sum_sin_I/n))/twopi;
    double phase_shift = (angleV-angleI+180) - (360*floor((angleV-angleI+180) / 360.0)) - 180;

    memset(proc, 0, sizeof(Accum));
    
    Serial.print(Vrms);
    Serial.print(' ');
    Serial.print(Irms);
    Serial.print(' ');
    Serial.print(activePower);
    Serial.print(' ');
    Serial.print(apparentPower);
    Serial.print(' ');
    Serial.print(powerFactor,3);
    Serial.print(' ');
    Serial.print(power_factor_degrees,3);
    Serial.print(' ');
    Serial.print(-phase_shift,2);
    Serial.print(" | ");
    Serial.print(power_factor_degrees-samplePhaseOffset,2);
    Serial.print(' ');
    Serial.print(-phase_shift-samplePhaseOffset,2);
    Serial.println(" | ");
    delay(50);
  }
}

ISR(ADC0_RESRDY_vect) 
{
  ADC0.INTFLAGS = ADC_RESRDY_bm;
  PORTE.OUT |= PIN2_bm;

  static int16_t voltage,current,sinval,cosval;
  static uint16_t n;

  int16_t volatile rawSample;
  rawSample = ADC0.RES;

  next = muxpos + 2;
  if (next>1) next -= 2;
  
  ADC0.MUXPOS = ADC_Sequence[next];

  if (muxpos==0) {
    // significantly faster assigning n here
    n = (int)coll->sampleSets%sampleSetsPerPeriod; 
    
    voltage = rawSample - (ADC_Counts >> 1);
    
    coll->cumV_deltas += voltage;
    coll->sum_Vsquared += ((long)voltage * voltage);

    sinval = sin_values[n];
    cosval = cos_values[n];

    coll->sum_sin_V += (long)voltage * sinval;
    coll->sum_cos_V += (long)voltage * cosval;
    
  } else if (muxpos==1) {
    current = rawSample - (ADC_Counts >> 1);
    
    coll->cumI_deltas += current;
    
    coll->sum_Isquared += ((long)current * current);

    coll->sumP += ((long)voltage * current);

    coll->sum_sin_I += (long)current * sinval;
    coll->sum_cos_I += (long)current * cosval;

    coll->sampleSets++;

    if (coll->sampleSets>sampleSetsToCapture) {
      accumSwap(&coll, &proc);
      reading_ready = true;
    }
  }
  
  muxpos++;
  if (muxpos>1) muxpos = 0;
  PORTE.OUT &=~PIN2_bm;

}
