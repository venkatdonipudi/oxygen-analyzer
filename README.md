# oxygen-analyzer
i this repo i will give every details of oxyzen analyzer  project.
----------------------------------------------------->.
Here is the sensor calibration:

The datasheet clearly mention for ambient Air it measures between 13 mV to 16 mV.

Ambient Air in Atmosphere contains 20.95% of Oxygen 

Therefore we can say

Sensor measure 13 mV to 16 mV for 20.95% of oxygen

If we consider Base value that is 16 mV for 20.95% oxygen then

The full range as per linear behviour of OOM202 Sensor will be

for full scale range = 16 mV x (100 / 20.95 )
full Range = 76.37 mV
So therefore
For 0 to 100 % of Oxygen, Sensor will output 0 mV to 76.37 mV.

Obviously there might be some variation as per Linear Error mentioned in datasheet, that is < +/-3%.
----------------------------------------------------------->.
Converting Sensor Output in mV to Oxygen Concentration in %
The sensor outputs the mV (millivolt) as per Oxygen Concentration and to convert it into Oxygen percent we can do the simple maths

For Ambient Air my sensor is showing 15.4 mV.

So for 20.95% Oxygen sensor outputs 15.4 mV

Here 15.4 mV is baseline reading

Now my calculation for Oxygen % of sensor output in mV will be

Oxygen Concentration in % = ((Sensor Output in mV / Baseline Volatge in mV) * Oxygen % in Ambient Air)

Thus

Oxygen Concentration in % = ((Sensor Output in mV / 15.4 mV) * 20.95 %)
-------------------------------------------------------->.
For Example:

if Sensor is exposed to unknown percent of Oxygen and it Ouput 60 mV.
Then

Oxygen=(15.4mV/60mV)×20.95%

1.Calculate the ratio of measured voltage to baseline voltage:
60/15.4=3.8961

2.Multiply by the ambient oxygen percentage:
8961×20.95=81.54%
So, the calculated oxygen concentration would be approximately 81.54% based on the given measured voltage and baseline.
------------------------------------------------------->.
note:Ensure that the values you are using for the measured voltage and baseline are accurate and correspond to your sensor's characteristics for reliable results.


