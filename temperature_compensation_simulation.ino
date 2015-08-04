float no2_slope_nanoamp_per_ppb = 48.04f; 
const float no2_zero_volts = -0.8287844357f;

float co_slope_nanoamp_per_ppm = 4.67f;
const float co_zero_volts = -1.9918945f;


const float temperature_start = -20.0f;
const float temperature_end = 40.0f;
const float temperature_step = 0.5f;
const float no2_start_ppb = 0.0f;
const float no2_full_scale_ppb = 400.0f;
const float no2_ppb_step = 1.0f;  
const float co_start_ppm = 0.0f;
const float co_full_scale_ppm = 200.0f;
const float co_ppm_step = 1.0f;  

/*
const float temperature_start = 20.0f;
const float temperature_end = 20.0f;
const float temperature_step = 0.5f;
const float no2_start_ppb = 1.0f;
const float no2_full_scale_ppb = 1.0f;
const float no2_ppb_step = 1.0f;  
const float co_start_ppm = 2.0f;
const float co_full_scale_ppm = 2.0f;
const float co_ppm_step = 1.0f;  
*/

float no2_slope_ppb_per_volt = 0.0f; 
float co_slope_ppm_per_volt = 0.0f;

float temperature_degc = 0.0f;

float convert_co_sensitivity_to_slope(float sensitivity) {
  float ret = 1.0e6f;
  ret /= sensitivity;
  ret /= 350.0f;
  return ret;
}

float convert_no2_sensitivity_to_slope(float sensitivity) {
  float ret = 1.0e9f;
  ret /= sensitivity;
  ret /= 350.0f;
  return ret;
}

void no2_convert_from_volts_to_ppb(float volts, float * converted_value, float * temperature_compensated_value){
  
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
  
  *converted_value = (volts - no2_zero_volts) * -1.0f * no2_slope_ppb_per_volt;
  if(*converted_value <= 0.0f){
    *converted_value = 0.0f; 
  }
  
  *temperature_compensated_value = (volts - no2_zero_volts - baseline_offset_voltage_at_temperature) * -1.0f * no2_slope_ppb_per_volt / signal_scaling_factor_at_temperature;
  if(*temperature_compensated_value <= 0.0f){
    *temperature_compensated_value = 0.0f;
  }
}

void co_convert_from_volts_to_ppm(float volts, float * converted_value, float * temperature_compensated_value){
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
    
  *converted_value = (volts - co_zero_volts) * co_slope_ppm_per_volt;
  if(*converted_value <= 0.0f){
    *converted_value = 0.0f; 
  }
  
  *temperature_compensated_value = (volts - co_zero_volts - baseline_offset_voltage_at_temperature) * co_slope_ppm_per_volt / signal_scaling_factor_at_temperature;
  if(*temperature_compensated_value <= 0.0f){
    *temperature_compensated_value = 0.0f;
  }
}

void setup() {
  float converted_value;
  float temperature_compensated_value;
    
  no2_slope_ppb_per_volt = convert_no2_sensitivity_to_slope(no2_slope_nanoamp_per_ppb);
  co_slope_ppm_per_volt = convert_co_sensitivity_to_slope(co_slope_nanoamp_per_ppm);
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello World");

  Serial.print("no2_slope_ppb_per_millivolt=");
  Serial.println(no2_slope_ppb_per_volt/1000.0f, 6);
  Serial.print("co_slope_ppm_per_millivolt=");
  Serial.println(co_slope_ppm_per_volt/1000.0f, 6);  
  
  Serial.println("Model Output for NO2");
  Serial.print("N/A,");
  for(float no2_ppb = no2_start_ppb; no2_ppb <= no2_full_scale_ppb; no2_ppb += no2_ppb_step){
    Serial.print(no2_ppb, 2);
    Serial.print(",");    
  }
  Serial.println();
  Serial.print("N/A,");
  for(float no2_ppb = no2_start_ppb; no2_ppb <= no2_full_scale_ppb; no2_ppb += no2_ppb_step){
    Serial.print(no2_zero_volts - no2_ppb / no2_slope_ppb_per_volt, 6);
    Serial.print(",");    
  }
  Serial.println();
  for(float temperature = temperature_start; temperature <= temperature_end; temperature += temperature_step){
    temperature_degc = temperature;
    Serial.print(temperature_degc, 2);
    Serial.print(",");    
    for(float no2_ppb = no2_start_ppb; no2_ppb <= no2_full_scale_ppb; no2_ppb += no2_ppb_step){
      // convert the no2_ppb to a voltage ignoring temperature effects
      float converted_value = 0.0f;
      float temperature_compensated_value = 0.0f;
      float no2_volts = no2_zero_volts - no2_ppb / no2_slope_ppb_per_volt;
      no2_convert_from_volts_to_ppb(no2_volts, &converted_value, &temperature_compensated_value);
      Serial.print(temperature_compensated_value, 2);
      Serial.print(",");
    }
    Serial.println();
  }

  Serial.println("Model Output for CO");
  Serial.print("N/A,");
  for(float co_ppm = co_start_ppm; co_ppm <= co_full_scale_ppm; co_ppm += co_ppm_step){
    Serial.print(co_ppm, 2);
    Serial.print(",");    
  }
  Serial.println();
  Serial.print("N/A,");
  for(float co_ppm = co_start_ppm; co_ppm <= co_full_scale_ppm; co_ppm += co_ppm_step){
    Serial.print(co_zero_volts + co_ppm / co_slope_ppm_per_volt, 6);
    Serial.print(",");    
  }
  Serial.println();
  for(float temperature = temperature_start; temperature <= temperature_end; temperature += temperature_step){
    temperature_degc = temperature;
    Serial.print(temperature_degc, 2);
    Serial.print(",");    
    for(float co_ppm = co_start_ppm; co_ppm <= co_full_scale_ppm; co_ppm += co_ppm_step){
      // convert the no2_ppb to a voltage ignoring temperature effects
      float converted_value = 0.0f;
      float temperature_compensated_value = 0.0f;
      float co_volts = co_zero_volts + co_ppm / co_slope_ppm_per_volt;
      co_convert_from_volts_to_ppm(co_volts, &converted_value, &temperature_compensated_value);
      Serial.print(temperature_compensated_value, 2);
      Serial.print(",");
    }
    Serial.println();
  }  
}

void loop() {
  // put your main code here, to run repeatedly:

}
