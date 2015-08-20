import sys

# require four arguments
# the zero calibration voltage
# the sensor slope in nA/ppm
# the altitude of the sensor
# NO2 or CO [to pick the temperature effect curves]
# the filename with the simulation data
#    CSV data format: voltage in mV, internal temperature degC

idx_no2_baseline = 1
idx_co_baseline  = 2
idx_altitude     = 3
idx_sensor_type  = 4
idx_input_file   = 5

if len(sys.argv) != 6:
  print "Example Usage: python analyze.py -1.858223 5.23 1423 CO data.csv"
  exit()

altitude = float(sys.argv[idx_altitude])

zero_offset = float(sys.argv[idx_no2_baseline])
slope_nA_per_ppm = float(sys.argv[idx_co_baseline])

voltages = []
inside_temperatures = []
outside_temperatures = []

with open(sys.argv[idx_input_file], 'r') as f:
  f = f.read()  
  firstLine = True
  for line in f.split('\n'):
    values = line.split(',')
    if(len(values) > 3):
      voltages.append(float(values[0]) * 1e-6)
      inside_temperatures.append(float(values[1]))
      outside_temperatures.append(float(values[2]))
      
def pressure_signal_scaling(altitude, outside_temperature):
  kelvin_offset = 273.15
  lapse_rate_kelvin_per_meter = -0.0065
  pressure_exponentiation_constant = 5.2558774324

  outside_temperature_kelvin = kelvin_offset + outside_temperature
  outside_temperature_kelvin_at_sea_level = outside_temperature_kelvin - lapse_rate_kelvin_per_meter * altitude
  pow_arg = 1.0 + ((lapse_rate_kelvin_per_meter * altitude) / outside_temperature_kelvin_at_sea_level)
  return pow(pow_arg, pressure_exponentiation_constant)
  
def co_signal_scaling(temperature):
  return (-2.82615380012526e-5*temperature*temperature*temperature - 0.0011816627*temperature*temperature + 0.5964229261*temperature + 88.1457987553)/100.0

def no2_signal_scaling(temperature):
  return (-4.93424959242815e-6*temperature*temperature*temperature + 0.0048931731*temperature*temperature + 0.0600254621*temperature + 98.2722021609)/100.0

def no2_baseline_offset(temperature, slope_ppb_per_volt):
  no2_baseline_offset_ppm = 0
  if(temperature > 33):
    no2_baseline_offset_ppm = -3.99183341047149e-5*temperature*temperature*temperature + 0.0032409112*temperature*temperature - 0.0866090614*temperature + 0.7514426299
  else:
    no2_baseline_offset_ppm = -0.0005707102*temperature + 0.0157955648
  return (no2_baseline_offset_ppm * 1000) / slope_ppb_per_volt
  
def co_baseline_offset(temperature, slope_ppm_per_volt):
  co_baseline_offset_ppm = 0
  if temperature >= 15.5:  
    co_baseline_offset_ppm = -4.44629007757936e-5*temperature*temperature*temperature + 0.0230847418*temperature*temperature - 0.6778264583*temperature + 5.768809965
  return co_baseline_offset_ppm / slope_ppm_per_volt 

def no2_voltage(internal_temperature, ppb, slope_ppb_per_volt, altitude, external_temperature):
  #print "====="
  #print temperature
  #print ppb
  #print zero_offset + no2_baseline_offset(temperature, slope_ppb_per_volt)
  #print (ppb / slope_ppb_per_volt) 
  #print ((ppb / slope_ppb_per_volt) * no2_signal_scaling(temperature))
  #print zero_offset + no2_baseline_offset(temperature, slope_ppb_per_volt) - ((ppb / slope_ppb_per_volt) * no2_signal_scaling(temperature))
  
  return zero_offset + no2_baseline_offset(internal_temperature, slope_ppb_per_volt) - ((ppb / slope_ppb_per_volt) * no2_signal_scaling(internal_temperature) * pressure_signal_scaling(altitude, external_temperature))

def co_voltage(internal_temperature, ppm, slope_ppm_per_volt, altitude, external_temperature):
  #print "====="
  #print temperature
  #print ppm
  #print zero_offset + co_baseline_offset(temperature, slope_ppm_per_volt)
  #print (ppm / slope_ppm_per_volt)
  #print ((ppm / slope_ppm_per_volt) * co_signal_scaling(temperature))
  #print zero_offset + co_baseline_offset(temperature, slope_ppm_per_volt) + ((ppm / slope_ppm_per_volt) * co_signal_scaling(temperature))  
    
  return zero_offset + co_baseline_offset(internal_temperature, slope_ppm_per_volt) + ((ppm / slope_ppm_per_volt) * co_signal_scaling(internal_temperature) * pressure_signal_scaling(altitude, external_temperature))
  
def find_nearest_co_concentration(internal_temperature, external_temperature, voltage, slope_ppm_per_volt, altitude):
  ppm = 0.0
  first = True
  last_calculated_voltage = 0  
  calculated_voltage_at_zero_ppm = 0  
  while ppm < 4000:
    calculated_voltage = co_voltage(internal_temperature, ppm, slope_ppm_per_volt, altitude, external_temperature)
    if first:
      first = False
      calculated_voltage_at_zero_ppm = calculated_voltage
      if voltage < calculated_voltage_at_zero_ppm: # calculated voltage is only going to get bigger
        #print str(voltage) +" < " + str(calculated_voltage_at_zero_ppm)
        return 0
    else:
      if voltage > last_calculated_voltage and voltage <= calculated_voltage:
        if abs(voltage - last_calculated_voltage) < abs(voltage - calculated_voltage):
          return ppm - 0.1
        else:
          return ppm
    ppm = ppm + 0.1
    last_calculated_voltage = calculated_voltage        
    
  print "PANIC: " + str(voltage) + " " + str(calculated_voltage_at_zero_ppm) + " " + str(last_calculated_voltage)
  
def find_nearest_no2_concentration(internal_temperature, external_temperature, voltage, slope_ppb_per_volt, altitude):
  ppb = 0.0
  first = True
  last_calculated_voltage = 0
  calculated_voltage_at_zero_ppb = 0
  while ppb < 4000:
    calculated_voltage = no2_voltage(internal_temperature, ppb, slope_ppb_per_volt, altitude, external_temperature)  
    if first:
      first = False  
      calculated_voltage_at_zero_ppb = calculated_voltage
      if voltage > calculated_voltage_at_zero_ppb: # calculated voltage is only going to get smaller
        return 0  
    else:
      #print str(last_calculated_voltage) + " " + str(voltage) + " " + str(calculated_voltage)    
      if voltage > calculated_voltage and voltage <= last_calculated_voltage:
        if abs(voltage - last_calculated_voltage) < abs(voltage - calculated_voltage):
          return ppb - 0.1
        else:
          return ppb
    ppb = ppb + 0.1
    last_calculated_voltage = calculated_voltage        
  
  print "PANIC: " + str(voltage) + " " + str(calculated_voltage_at_zero_ppb) + " " + str(last_calculated_voltage)
  
if sys.argv[idx_sensor_type] == "CO":
  slope_ppm_per_volt = 1.0e6 / slope_nA_per_ppm / 350.0    
  for i in range(len(inside_temperatures)):
    print find_nearest_co_concentration(inside_temperatures[i], outside_temperatures[i], voltages[i], slope_ppm_per_volt, altitude)      
else:
  slope_ppb_per_volt = 1.0e9 / slope_nA_per_ppm / 350.0
  for i in range(len(inside_temperatures)):
    print find_nearest_no2_concentration(inside_temperatures[i], outside_temperatures[i], voltages[i], slope_ppb_per_volt, altitude)

