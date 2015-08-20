boolean is_no2 = true;
boolean is_co = false;

int16_t altitude_meters = 1459;
float reported_temperature_offset_degC = 2.95f;

float no2_zero_volts = -0.828167629f;
float no2_sensitivity = 37.09f;

float co_zero_volts = 0.0f;
float co_sensitivity = 0.0f;

float data[][2] = {
{-0.8285,28.59},
{-0.82849,28.6},
{-0.82849,28.6},
{-0.82848,28.61},
{-0.82848,28.61},
{-0.82849,28.62},
{-0.82849,28.62},
{-0.82847,28.63},
{-0.82848,28.63},
{-0.8285,28.64},
{-0.8285,28.64},
{-0.8285,28.65},
{-0.82851,28.65},
{-0.82852,28.66},
{-0.82852,28.67},
{-0.82853,28.67},
{-0.82854,28.68},
{-0.82854,28.68},
{-0.82852,28.69},
{-0.82851,28.69},
{-0.82849,28.7},
{-0.82849,28.71},
{-0.82847,28.71},
{-0.82847,28.72},
{-0.82847,28.72},
{-0.82848,28.73},
{-0.82849,28.73},
{-0.82854,29.18},
{-0.82854,29.18},
{-0.82853,29.19},
{-0.82858,29.38},
{-0.82858,29.38},
{-0.82858,29.39},
{-0.82857,29.39},
{-0.82857,29.39},
{-0.82858,29.4},
{-0.82859,29.4},
{-0.82858,29.4},
{-0.82858,29.41},
{-0.82855,29.41},
{-0.82856,29.42},
{-0.82855,29.42},
{-0.82854,29.43},
{-0.82854,29.43},
{-0.82853,29.43},
{-0.82851,29.44},
{-0.8285,29.44},
{-0.82851,29.44},
{-0.82849,29.45},
{-0.82849,29.45},
{-0.82847,29.45},
{-0.82846,29.45},
{-0.82845,29.46},
{-0.82843,29.46},
{-0.82842,29.46},
{-0.82842,29.46},
{-0.82843,29.46},
{-0.82845,29.46},
{-0.82844,29.47},
{-0.82845,29.47},
{-0.82843,29.47},
{-0.82844,29.47},
{-0.82844,29.47},
{-0.82843,29.47},
{-0.82843,29.48},
{-0.82844,29.48},
{-0.82847,29.48},
{-0.82846,29.48},
{-0.82846,29.48},
{-0.82847,29.48},
{-0.82848,29.48},
{-0.82851,29.48},
{-0.82852,29.49},
{-0.82852,29.49},
{-0.82852,29.49},
{-0.82852,29.49},
{-0.82853,29.49},
{-0.82853,29.49},
{-0.82855,29.49},
{-0.82855,29.5},
{-0.82856,29.5},
{-0.82856,29.5},
{-0.82859,29.5},
{-0.82858,29.5},
{-0.82859,29.5},
{-0.8286,29.5},
{-0.82861,29.51},
{-0.82861,29.51},
{-0.8286,29.51},
{-0.8286,29.51},
{-0.82865,29.59},
{-0.82865,29.6},
{-0.82864,29.6},
{-0.82864,29.6},
{-0.82864,29.61},
{-0.82863,29.61},
{-0.82863,29.62},
{-0.82863,29.62},
{-0.82864,29.62},
{-0.82862,29.63},
{-0.82865,29.63},
{-0.82854,29.53},
{-0.82856,29.54},
{-0.82858,29.54},
{-0.82859,29.55},
{-0.8286,29.55},
{-0.8286,29.55},
{-0.8286,29.56},
{-0.82861,29.56},
{-0.8286,29.57},


  {0.0,0.0}
};

// don't change the following
float temperature_degc = 0.0f;
float no2_slope_ppb_per_volt = 0.0f;
float co_slope_ppm_per_volt = 0.0f;

void setup(){
  Serial.begin(115200);
  Serial.println("Hello World");

  no2_slope_ppb_per_volt = convert_no2_sensitivity_to_slope(no2_sensitivity);
  co_slope_ppm_per_volt = convert_co_sensitivity_to_slope(co_sensitivity);

  uint16_t ii = 0;
  while(data[ii][0] != 0.0f){
    float converted_value = 0.0f;
    float temperature_compensated_value = 0.0f;
    float voltage = 0.0f;

    voltage          = data[ii][0];
    temperature_degc = data[ii][1];        

    if(is_no2){
      no2_convert_from_volts_to_ppb(voltage, &converted_value, &temperature_compensated_value);
      
    }
    else{
      co_convert_from_volts_to_ppm(voltage, &converted_value, &temperature_compensated_value);
    }
    
    Serial.println(temperature_compensated_value, 2);
    
    ii++;
  }
  
}

void loop(){

}

void no2_convert_from_volts_to_ppb(float volts, float * converted_value, float * temperature_compensated_value){
  float temperature_coefficient_of_span = 0.0f;
  float temperature_compensated_slope = 0.0f;
  
  // apply piecewise linear regressions
  // to signal scaling effect curve
  float scaling_slope = 0.0f;
  float scaling_intercept = 0.0f;  
  if(temperature_degc < 0.0f){                 // < 0C  
    scaling_slope = -0.0355739076f;
    scaling_intercept = 97.9865525718f;
  }
  else if(temperature_degc < 20.0f){           // 0C .. 20C   
    scaling_slope = 0.1702484721f;
    scaling_intercept = 97.9953985672f; 
  }
  else{                                        // > 20C   
    scaling_slope = 0.3385634354f;
    scaling_intercept = 94.6638669473f;
  }
  float signal_scaling_factor_at_temperature = ((scaling_slope * temperature_degc) + scaling_intercept)/100.0f;
  // divide by 100 becauset the slope/intercept graphs have scaling factors in value

  // apply piecewise linear regressions
  // to baseline offset effect curve
  float baseline_offset_ppb_slope = 0.0f;
  float baseline_offset_ppb_intercept = 0.0f;
                                                                     
  if(temperature_degc < 33.0f){                          // < 33C
    baseline_offset_ppb_slope = -0.0007019288f;
    baseline_offset_ppb_intercept = 0.0177058403f;
  }
  else if(temperature_degc < 38.0f){                     // 33C .. 38C
    baseline_offset_ppb_slope = -0.0085978946f;
    baseline_offset_ppb_intercept = 0.2777254052f;
  }
  else if(temperature_degc < 42.0f){                     // 38C .. 42C
    baseline_offset_ppb_slope = -0.0196092331f;
    baseline_offset_ppb_intercept = 0.6994563331f;    
  }
  else if(temperature_degc < 46.0f){                     // 42C .. 46C
    baseline_offset_ppb_slope = -0.0351416006f;
    baseline_offset_ppb_intercept = 1.3566041809f;          
  }
  else{                                                  // > 46C
    baseline_offset_ppb_slope = -0.0531894279f;
    baseline_offset_ppb_intercept =  2.1948987152f;        
  }  
  float baseline_offset_ppm_at_temperature = ((baseline_offset_ppb_slope * temperature_degc) + baseline_offset_ppb_intercept); 
  float baseline_offset_ppb_at_temperature = baseline_offset_ppm_at_temperature * 1000.0f;
  // multiply by 1000 because baseline offset graph shows NO2 in ppm  
  float baseline_offset_voltage_at_temperature = baseline_offset_ppb_at_temperature / no2_slope_ppb_per_volt;

  float signal_scaling_factor_at_altitude = pressure_scale_factor();
  
  *converted_value = (volts - no2_zero_volts) * -1.0f * no2_slope_ppb_per_volt;
  if(*converted_value <= 0.0f){
    *converted_value = 0.0f; 
  }
  
  *temperature_compensated_value = (volts - no2_zero_volts - baseline_offset_voltage_at_temperature) * -1.0f * no2_slope_ppb_per_volt 
                                   / signal_scaling_factor_at_temperature 
                                   / signal_scaling_factor_at_altitude;
  if(*temperature_compensated_value <= 0.0f){
    *temperature_compensated_value = 0.0f;
  }
}

void co_convert_from_volts_to_ppm(float volts, float * converted_value, float * temperature_compensated_value){  
  float temperature_coefficient_of_span = 0.0f;
  float temperature_compensated_slope = 0.0f;  

  // apply piecewise linear regressions
  // to signal scaling effect curve
  float scaling_slope = 0.0f;
  float scaling_intercept = 0.0f;  
  if(temperature_degc < 0.0f){       // < 0C
    scaling_slope = 0.926586438f;
    scaling_intercept = 88.2942019565f;
  }
  else if(temperature_degc < 20.0f){ // 0C .. 20C
    scaling_slope = 0.6072408915f;
    scaling_intercept = 87.9176593244f; 
  }
  else{                              // > 20C
    scaling_slope = 0.2600853674f;
    scaling_intercept = 95.6168149016f;
  }
  float signal_scaling_factor_at_temperature = ((scaling_slope * temperature_degc) + scaling_intercept)/100.0f;
  // divide by 100 becauset the slope/intercept graphs have scaling factors in value
  
  // apply piecewise linear regressions
  // to baseline offset effect curve
  float baseline_offset_ppm_slope = 0.0f;
  float baseline_offset_ppm_intercept = 0.0f;
                
  if(temperature_degc < 15.5f){                          // no correction for < 15.5C
    baseline_offset_ppm_slope = 0.0f;
    baseline_offset_ppm_intercept = 0.0f;
  }
  else if(temperature_degc < 25.0f){                      // 15.5C .. 25C
    baseline_offset_ppm_slope = 0.2590260005f;
    baseline_offset_ppm_intercept = -4.0290395187f;
  }
  else if(temperature_degc < 32.0f){                      // 25C .. 32C
    baseline_offset_ppm_slope = 0.5387700048f;
    baseline_offset_ppm_intercept = -11.0899532317f;
  }
  else{                                                   // > 32C
    baseline_offset_ppm_slope = 0.824964228f;
    baseline_offset_ppm_intercept = -20.3665881995f;        
  }  
  float baseline_offset_ppm_at_temperature = (baseline_offset_ppm_slope * temperature_degc) + baseline_offset_ppm_intercept;  
  float baseline_offset_voltage_at_temperature = baseline_offset_ppm_at_temperature / co_slope_ppm_per_volt;

  float signal_scaling_factor_at_altitude = pressure_scale_factor();
    
  *converted_value = (volts - co_zero_volts) * co_slope_ppm_per_volt;
  if(*converted_value <= 0.0f){
    *converted_value = 0.0f; 
  }
  
  *temperature_compensated_value = (volts - co_zero_volts - baseline_offset_voltage_at_temperature) * co_slope_ppm_per_volt 
                                   / signal_scaling_factor_at_temperature
                                   / signal_scaling_factor_at_altitude;
  if(*temperature_compensated_value <= 0.0f){
    *temperature_compensated_value = 0.0f;
  }
}

float pressure_scale_factor(void){
  float ret = 1.0f;
  
  if(altitude_meters != -1){
    // calculate scale factor of altitude and temperature
    const float kelvin_offset = 273.15f;
    const float lapse_rate_kelvin_per_meter = -0.0065f;
    const float pressure_exponentiation_constant = 5.2558774324f;
    
    float outside_temperature_kelvin = kelvin_offset + (temperature_degc - reported_temperature_offset_degC);
    float outside_temperature_kelvin_at_sea_level = outside_temperature_kelvin - lapse_rate_kelvin_per_meter * altitude_meters; // lapse rate is negative
    float pow_arg = 1.0f + ((lapse_rate_kelvin_per_meter * altitude_meters) / outside_temperature_kelvin_at_sea_level);
    ret = powf(pow_arg, pressure_exponentiation_constant);
  }
  
  return ret;
}

float convert_no2_sensitivity_to_slope(float sensitivity) {
  float ret = 1.0e9f;
  ret /= sensitivity;
  ret /= 350.0f;
  return ret;
}

float convert_co_sensitivity_to_slope(float sensitivity) {
  float ret = 1.0e6f;
  ret /= sensitivity;
  ret /= 350.0f;
  return ret;
}

