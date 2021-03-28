import serial
import time
import csv
import numpy as np
import cv2
from PIL import Image

# Global
num = 0


def display_photo():
    with open('data_for_photo_0.csv', 'r') as f:
        rgb565array = np.genfromtxt(f, delimiter=',').astype(np.uint16)

    # Pick up image dimensions
    h, w = rgb565array.shape

    # Make a numpy array of matching shape, but allowing for 8-bit/channel for R, G and B
    rgb888array = np.zeros([h, w, 3], dtype=np.uint8)

    for row in range(h):
        for col in range(w):
            # Pick up rgb565 value and split into rgb888
            rgb565 = rgb565array[row, col]
            r = ((rgb565 >> 11) & 0x1f) << 3
            g = ((rgb565 >> 5) & 0x3f) << 2
            b = (rgb565 & 0x1f) << 3
            # Populate result array
            rgb888array[row, col] = r, g, b

    # Save result as PNG
    Image.fromarray(rgb888array).save('result.png')

    print("Image Created")

    img = cv2.imread("result.png")
    img = cv2.flip(img, 1)
    cv2.imshow("Output", img)
    cv2.waitKey(0)


def add_to(ver, data, type_):
    open("data_for_photo_" + str(ver) + ".csv", type_).write(data)


def collect_the_data():
    check_ = True
    count = 0

    while check_:

        #
        # Collect 80 bytes at a time until the sender is done.
        # Collect the vh, vl values
        #

        ser_bytes_1_vh = ser.read(80)  # 80 bytes at a time is a safe number
        ser_bytes_1_vl = ser.read(80)

        ser_bytes_2_vh = ser.read(80)
        ser_bytes_2_vl = ser.read(80)

        ser_bytes_3_vh = ser.read(80)
        ser_bytes_3_vl = ser.read(80)

        ser_bytes_4_vh = ser.read(80)
        ser_bytes_4_vl = ser.read(80)

        ser.write(b'b')

        decode_bytes_all_vh = list(ser_bytes_1_vh) + list(ser_bytes_2_vh) + list(ser_bytes_3_vh) + list(ser_bytes_4_vh)
        decode_bytes_all_vl = list(ser_bytes_1_vl) + list(ser_bytes_2_vl) + list(ser_bytes_3_vl) + list(ser_bytes_4_vl)

        data_vh = np.array(decode_bytes_all_vh, dtype=np.uint8)
        data_vl = np.array(decode_bytes_all_vl, dtype=np.uint8)

        # print(data_vh)
        # print(data_vl)

        if ser_bytes_1_vh == b'done\r\n':  # Checks for needed code.
            check_ = False
        else:
            decode_combine = np.zeros([320])
            for x in range(320):
                decode_combine[x] = (data_vh[x] << 8) | (data_vl[x] & 0xff)

            # print(decode_combine)

            count = count + 1
            val = (100*count/240)
            print(("%4.1f" % val) + '%')
            with open('data_for_photo_0.csv', 'a+', newline='') as f:
                write = csv.writer(f)
                write.writerow(decode_combine)


def start_camera():
    add_to(num, '', "w")  # creates a new txt file 'w'

    ser.write(b'a')

    try:
        check = True
        while check:
            ser_bytes = ser.readline()
            decode_bytes = ser_bytes.decode("ascii")
            if decode_bytes == "Start\r\n":
                print("Found Entry Code")
                collect_the_data()
                check = False

        display_photo()

        print('done')

    except KeyboardInterrupt:
        ser.close()
        print('done')
        time.sleep(1)


if __name__ == '__main__':

    print('Opening port')
    ser = serial.Serial('COM5', 115200, timeout=1)

    print('Sending Byte')
    time.sleep(1)
    while True:
        try:
            start_camera()
        except KeyboardInterrupt:
            ser.close()
            time.sleep(1)
            quit()

