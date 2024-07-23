import minimalmodbus
import serial
import sys
import time

port = "COM4"
slave_id = 1

try: 
    instrument = minimalmodbus.Instrument(port=port,slaveaddress=slave_id)
    print("Instrument created")
    instrument.serial.baudrate = 9600
    instrument.serial.bytesize = 8
    instrument.serial.parity = serial.PARITY_NONE
    instrument.serial.stopbits = 1
except:
    print(f"Could not connect to device via {port}")
    sys.exit(1)

if __name__ == "__main__":
    while(True):
        try:
            data = instrument.read_registers(0,6,3)
            print(f"Temperature: {data[0]/10}C")
            print(f"Humility: {data[1]/10}%")
        except minimalmodbus.NoResponseError:
            print("\nRequest will fail on first poll\n")
            
        time.sleep(2)
        
    