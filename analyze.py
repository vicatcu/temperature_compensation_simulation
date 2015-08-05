import sys

# require four arguments
# the zero calibration voltage
# the sensor slope in nA/ppm
# NO2 or CO [to pick the temperature effect curves]
# the filename with the simulation data
#    first row gives the voltages
#    each row starts with a temperature_units, 
#       followed by a list of concentrations at the (temperature, voltage) intersection

if len(sys.argv) != 5 and len(sys.argv) != 6:
  print "Example Usage: python analyze.py -1.858223 5.23 CO data.csv"
  print "Example Usage: python analyze.py -1.858223 5.23 CO data.csv output_file_prefix"
  exit()

output_file_prefix = "out"
if len(sys.argv) == 6:
  output_file_prefix = sys.argv[5]

zero_offset = float(sys.argv[1])
slope_nA_per_ppm = float(sys.argv[2])

results = {}
voltages = []
temperatures = []
with open(sys.argv[4], 'r') as f:
  f = f.read()  
  firstLine = True
  for line in f.split('\n'):
    values = line.split(',')
    if(len(values) > 1):
      if firstLine:
        firstLine = False
        for i in range(1, len(values)):       
          if values[i] == "":
            break             
          voltages.append(float(values[i]))
      else:
        temperature = float(values[0])
        temperatures.append(temperature)
        results[temperature] = {}    
        for i in range(1, len(values)): 
          if values[i] == "":
            break        
          voltage = float(values[i])        
          results[temperature][voltages[i-1]] = {}
          results[temperature][voltages[i-1]]["simulated"] = voltage
  
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

def no2_voltage(temperature, ppb, slope_ppb_per_volt):
  #print "====="
  #print temperature
  #print ppb
  #print zero_offset + no2_baseline_offset(temperature, slope_ppb_per_volt)
  #print (ppb / slope_ppb_per_volt) 
  #print ((ppb / slope_ppb_per_volt) * no2_signal_scaling(temperature))
  #print zero_offset + no2_baseline_offset(temperature, slope_ppb_per_volt) - ((ppb / slope_ppb_per_volt) * no2_signal_scaling(temperature))
  
  return zero_offset + no2_baseline_offset(temperature, slope_ppb_per_volt) - ((ppb / slope_ppb_per_volt) * no2_signal_scaling(temperature))

def co_voltage(temperature, ppm, slope_ppm_per_volt):
  #print "====="
  #print temperature
  #print ppm
  #print zero_offset + co_baseline_offset(temperature, slope_ppm_per_volt)
  #print (ppm / slope_ppm_per_volt)
  #print ((ppm / slope_ppm_per_volt) * co_signal_scaling(temperature))
  #print zero_offset + co_baseline_offset(temperature, slope_ppm_per_volt) + ((ppm / slope_ppm_per_volt) * co_signal_scaling(temperature))  
    
  return zero_offset + co_baseline_offset(temperature, slope_ppm_per_volt) + ((ppm / slope_ppm_per_volt) * co_signal_scaling(temperature))
  
def find_nearest_co_concentration(temperature, voltage, slope_ppm_per_volt):
  ppm = 0.0
  first = True
  last_calculated_voltage = 0  
  calculated_voltage_at_zero_ppm = 0  
  while ppm < 4000:
    calculated_voltage = co_voltage(temperature, ppm, slope_ppm_per_volt)
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
  
def find_nearest_no2_concentration(temperature, voltage, slope_ppb_per_volt):
  ppb = 0.0
  first = True
  last_calculated_voltage = 0
  calculated_voltage_at_zero_ppb = 0
  while ppb < 4000:
    calculated_voltage = no2_voltage(temperature, ppb, slope_ppb_per_volt)  
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
  
if sys.argv[3] == "CO":
  slope_ppm_per_volt = 1.0e6 / slope_nA_per_ppm / 350.0  
  for temperature in temperatures:
    print temperature
    for voltage in voltages:
      results[temperature][voltage]["expected"] = find_nearest_co_concentration(temperature, voltage, slope_ppm_per_volt)
      results[temperature][voltage]["diff"] = results[temperature][voltage]["expected"] - results[temperature][voltage]["simulated"]
      results[temperature][voltage]["absdiff"] = abs(results[temperature][voltage]["diff"])
else:
  slope_ppb_per_volt = 1.0e9 / slope_nA_per_ppm / 350.0
  for temperature in temperatures:
    print temperature
    for voltage in voltages:
      results[temperature][voltage]["expected"] = find_nearest_no2_concentration(temperature, voltage, slope_ppb_per_volt)
      results[temperature][voltage]["diff"] = results[temperature][voltage]["expected"] - results[temperature][voltage]["simulated"]
      results[temperature][voltage]["absdiff"] = abs(results[temperature][voltage]["diff"])

with open(output_file_prefix + "_diff.csv", 'w') as f:
  f.write("N/A,")
  for voltage in voltages:  
    f.write(str(voltage) + ",")
  f.write("\r\n")
  
  for temperature in temperatures:
    f.write(str(temperature) + ",")
    for voltage in voltages:     
        f.write(str(results[temperature][voltage]["diff"]) + ",")
    f.write("\r\n")
    
  f.close()    

with open(output_file_prefix + "_absdiff.csv", 'w') as f:
  f.write("N/A,")
  for voltage in voltages:  
    f.write(str(voltage) + ",")
  f.write("\r\n")
  
  for temperature in temperatures:
    f.write(str(temperature) + ",")
    for voltage in voltages:     
        f.write(str(results[temperature][voltage]["absdiff"]) + ",")
    f.write("\r\n")
    
  f.close()  

with open(output_file_prefix + "_expected.csv", 'w') as f:
  f.write("N/A,")
  for voltage in voltages:  
    f.write(str(voltage) + ",")
  f.write("\r\n")
  
  for temperature in temperatures:
    f.write(str(temperature) + ",")
    for voltage in voltages:    
        f.write(str(results[temperature][voltage]["expected"]) + ",")
    f.write("\r\n")
    
  f.close()    
  