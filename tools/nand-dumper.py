import usb
import struct
import sys
import os
from pathlib import Path


def write_file(range_size, in_ep):
    with open("nand.bin", "wb") as f:
        while range_size:
            buf_size = 0x800000
            if (range_size - buf_size < 0):
                buf_size = range_size
            buf = in_ep.read(buf_size, timeout=0)
            f.write(buf)
            range_size -= buf_size
            print("writing\n")


def wait_for_input(in_ep, out_ep):
    print("now waiting for intput\n")

    while True:
        # read-in data sent from switch.
        file_range_header = in_ep.read(0x30, timeout=0)

        #in_ep.
        mode = struct.unpack('<Q', file_range_header[0:8])[0]
        range_offset = struct.unpack('<Q', file_range_header[16:24])[0]
        range_size = struct.unpack('<Q', file_range_header[32:40])[0]
        print("mode = {} | range offset = {} | range size = {}".format(mode, range_offset, range_size))

        if (mode == 0):
            break
        write_file(range_size, in_ep)


if __name__ == '__main__':

    # Find the switch
    print("waiting for to find the switch...\n")
    dev = usb.core.find(idVendor=0x057E, idProduct=0x3000)

    while (dev is None):
        dev = usb.core.find(idVendor=0x057E, idProduct=0x3000)

    print("found the switch!\n")

    dev.reset()
    dev.set_configuration()
    cfg = dev.get_active_configuration()

    is_out_ep = lambda ep: usb.util.endpoint_direction(ep.bEndpointAddress) == usb.util.ENDPOINT_OUT
    is_in_ep = lambda ep: usb.util.endpoint_direction(ep.bEndpointAddress) == usb.util.ENDPOINT_IN
    out_ep = usb.util.find_descriptor(cfg[(0,0)], custom_match=is_out_ep)
    in_ep = usb.util.find_descriptor(cfg[(0,0)], custom_match=is_in_ep)

    assert out_ep is not None
    assert in_ep is not None

    wait_for_input(in_ep, out_ep)