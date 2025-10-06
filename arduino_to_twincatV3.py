import pyads
import serial
import re
import time


# --- Config ---
PLC_AMS_ID = '5.132.85.195.1.1'
PLC_PORT = 851
SERIAL_PORT = 'COM4'
BAUD_RATE = 9600

# ADS povezava
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
plc = pyads.Connection(PLC_AMS_ID, PLC_PORT)
plc.open()

print("Connection open. Listening to serial port...")

try:
     while True:
        line = ser.readline().decode().strip()

        # DEBUG izpis:
        if line:
            print("Serial input:", line)

        # Preveri ali vrstica vsebuje barvo in razred teže
        match = re.search(r'COLOR: (\d)\s*\|\s*WEIGHT CLASS: (\w+)', line)
        if match:
            color_id = int(match.group(1))  # 0 ali 1
            weight_class = match.group(2).lower()  # light, medium, heavy


            color = "white" if color_id == 0 else "black"

            # Izračunaj Bread_ID glede na barvo in težo
            bread_id = 0
            if color_id == 0 and weight_class == "heavy":
                bread_id = 1  # White Baguette
            elif color_id == 0 and weight_class == "medium":
                bread_id = 2  # White Toast
            elif color_id == 1 and weight_class == "medium":
                bread_id = 3  # Dark Rye
            elif color_id == 1 and weight_class == "light":
                bread_id = 4  # Dark Bagel

            plc.write_by_name("GVL.breadID", bread_id, pyads.PLCTYPE_INT)
            print(f"Sent to TwinCAT: breadID = {bread_id}")

            # Počakaj nekaj časa, nato počisti breadID (pusti TwinCAT-u da začne obdelavo)
            time.sleep(0.5)  # ali več, če je potrebno
            plc.write_by_name("GVL.breadID", 0, pyads.PLCTYPE_INT)

        # BIN full zaznava
        match_bin = re.search(r'Bin full: (\d)', line)
        if match_bin:
            bin_number = int(match_bin.group(1))
            plc.write_by_name("GVL.BinFullNumber", bin_number, pyads.PLCTYPE_INT)
except KeyboardInterrupt:
    print("Manual interruption.")
finally:
    ser.close()
    plc.close()
