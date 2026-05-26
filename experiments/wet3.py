import RPi.GPIO as GP
import time                        

GP.setmode(GP.BCM)
print('Test wet register')
GP.setup(4, GP.IN, pull_up_down=GP.PUD_DOWN)
GP.setup(17, GP.OUT)


GP.output(17, GP.LOW)
time.sleep(1)
pin_read = GP.input(4)

if pin_read == GP.HIGH :
    print("true")
else :
    print("false")




GP.cleanup()
