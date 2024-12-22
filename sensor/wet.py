import RPi.GPIO as GP
import time                        

GP.setmode(GP.BCM)
print('Test wet register')
GP.setup(17, GP.IN, pull_up_down=GP.PUD_DOWN)

time.sleep(1)
pin_read = GP.input(17)


if pin_read == GP.HIGH :
    print("true")
else :
    print("false")

GP.cleanup()
