#!/usr/bin/env python

import socket, sys, struct

from pypb.session_commands_pb2 import Command_Register as Reg
from pypb.commands_pb2 import CommandContainer as Cmd

HOST = "localhost"
PORT = 4747

def build_reg():
    cmd = Cmd()
    sc = cmd.session_command.add()

    reg = sc.Extensions[Reg.ext]
    reg.user_name = "testUser"
    reg.email = "test@example.com"
    reg.password = "password"

    cmd.cmd_id = 1
    return cmd

if __name__ == "__main__":
    print "Building registration command"
    r = build_reg()
    print "Attempting to register"

    address = (HOST, PORT)
    sock = socket.socket()
    sock.connect(address)

    # hack for old xml clients - server expects this and discards first message
    xmlClientHack = Cmd().SerializeToString()
    sock.sendall(struct.pack('H', len(xmlClientHack)))
    sock.sendall(xmlClientHack)

    print sock.recv(4096)

    msg = r.SerializeToString()
    packed = struct.pack('H', len(msg))
    sock.sendall(packed)
    sock.sendall(msg)

    resp = sock.recv(4096)
    print resp

