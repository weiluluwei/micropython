# main.py -- put your code here!

import pyb

def clock():
    h,m,s = 0, 0, 0
    while True:
        s= (s+1)%60
        if s==0:
            m = (m+1)%60
            if m==0:
                h = (h+1)
        print("%02d:%02d:%02d" % (h, m, s))
        pyb.delay(1000)


lcd = pyb.LCD()
x = y = 0.0
dx = 1.0
dy = 0.31831
lcd.fill(0)
cnt=0
xSize = 240
ySize = 320
lim = 0.3

while True:
    # update the dot's position
    x += dx
    y += dy

    # make the dot bounce of the edges of the screen
    if x <= 0 or x >= xSize:
        dx = -dx
    if y <= 0 or y >= ySize:
        dy = -dy
    frac = float(cnt)/(ySize*xSize)
    col = int(256.0*frac/lim)*2**16+256*int(256*(1.0-frac/lim))+int(256*(1.0-frac/lim))
    lcd.pixel(int(x), int(y), int(col))          # draw the dot
    pyb.delay(2)               # pause for 50ms
    cnt+=1
    if frac>lim:
        lcd.fill(0)
        cnt=0
    #lcd.pixel(x, y, 0)          # undraw the dot
    #lcd.write("%3.1f" % frac, 10,10, 1)


