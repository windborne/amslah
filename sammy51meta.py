import re

f = open("core/pio/samd51n20a.h").read()

tots = re.findall('PINMUX_(....)(.)_(\w+)_(\w+) ', f)
from pprint import pprint
#pprint(tots)

#PORT_PC13D_SERCOM6_PAD0

from collections import defaultdict
byP = defaultdict(list)
for pin, mux, per, pad in tots:
    byP[per].append((pin, ord(mux)-ord('A'), pad))

"""
ains = {}
for x in byP['ADC0']:
    if x[2].startswith('AIN'):
        ains[x[0]] = int(x[2][3:])
print('const uint8_t adc_ains[] = {', end='')
for l in 'ABC':
    for i in range(32):
        print(str(ains.get('P%s%02d'%(l,i), -1))+', ', end='')
print('}')
        
        
pprint([x for x in byP['ADC0'] if x[2].startswith('AIN')])
pprint(byP['ADC1'])
print(byP.keys())
exit()
"""

def tc2num(s):
    if s.startswith('TCC'):
        return 10 + int(s[3:])
    else:
        return int(s[2:])

defs = []
for s in range(8):
    ser = "SERCOM%d"%s
    bypad = defaultdict(list)
    data = byP[ser]
    for pin, mux, pad in data:
        bypad[pad].append((pin, mux))

    # uart has effectively only tx=PAD0 so might as well set txpo=0 always

    for pin0, mux0 in bypad['PAD0']:
        for npad in bypad:
            if npad == "PAD0": continue
            for pin1, mux1 in bypad[npad]:
                defs.append(("UART_SERCOM%d_%s_%s" % (s, pin0, pin1), {"txpo": 0, "rxpo": int(npad.replace("PAD","")), "sercom": s, "mux_tx": mux0, "mux_rx": mux1, "pin_tx": pin0, "pin_rx": pin1}))

    # for SPI, SCK is always PAD1
    for pin_sck, mux_sck in bypad['PAD1']:
        dopos = {0: "PAD0", 2: "PAD3"}

        for pad_miso in bypad:
            if pad_miso == "PAD1": continue
            for pin_miso, mux_miso in bypad[pad_miso]:
                for dopo in dopos:
                    for pin_mosi, mux_mosi in bypad[dopos[dopo]]:
                        if pad_miso == dopos[dopo]: continue
                        defs.append(("SPI_SERCOM%d_%s_%s_%s" % (s, pin_sck, pin_mosi, pin_miso),
                                    {
                                    "dipo": int(pad_miso.replace("PAD","")),
                                    "dopo": dopo,
                                    "pin_sck": pin_sck, "mux_sck": mux_sck,
                                    "pin_miso": pin_miso, "mux_miso": mux_miso,
                                    "pin_mosi": pin_mosi, "mux_mosi": mux_mosi,
                                    "sercom": s
                                    }))
            

pwmseen = set()
for src in ['TCC'+x for x in '01234'] + ['TC'+x for x in '01234567']:
    for pin, mux, pad in byP[src]:
        defs.append(("PWM_%s_%s"%(pin, src),
                    {"pin": pin, "timer": tc2num(src), "mux": mux, "output": int(pad[2:])}))
        nn = "PWM_%s"%pin
        if nn not in pwmseen:
            defs.append((nn, defs[-1][1]))
            pwmseen.add(nn)


_= """    uint8_t sercom;
    uint8_t pin_tx;
    uint8_t mux_tx;
    uint8_t pin_rx;
    uint8_t mux_rx;
    uint8_t txpo;
    uint8_t rxpo;
    uint8_t pin_sck;
    uint8_t mux_sck;
    uint8_t pin_mosi;
    uint8_t mux_mosi;
    uint8_t pin_miso;
    uint8_t mux_miso;
    uint8_t dipo;
    uint8_t dopo;
    uint32_t baud;
    uint8_t pin;
    uint8_t timer;
    uint8_t mux;
    uint8_t output;"""
order = []
for x in _.split("\n"):
    order.append(x.split(';')[0].split(' ')[-1])

 
seen = set()
dic = {}
ff = ""
for x, y in defs:
    print(x)
    if x in seen:
        print("Yo already seen", x, "as", dic[x], "now", y)
    ff += "#define %s " % x
    ss = []
    for m in order:
        if m in y:
            ss.append(".%s=%s"%(m, str(y[m])))
    ff += ", ".join(ss)
    ff += "\n"
    dic[x] = y

with open("core/samd51defs.h", "w") as f:
    f.write(ff)


#pprint(defs)
print(dict(defs)["UART_SERCOM5_PB16_PB17"])
print(dic["UART_SERCOM4_PB12_PB09"])
print("Got", len(defs), "total")
