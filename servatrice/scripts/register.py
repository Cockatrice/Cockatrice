#!/usr/bin/env python

from pypb.session_commands_pb2 import Command_Register as Reg
from pypb.commands_pb2 import CommandContainer as Cmd

if __name__ == "__main__":
    print "Attempting to register"
    reg = Reg()
    reg.user_name = "testUser"
    reg.email = "test@example.com"
    reg.password = "password"
    cmd = Cmd()
    sc = cmd.session_command.add()
    sc.MESSAGE = r.EXT_FIELD_NUMBER
