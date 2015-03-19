#!/usr/bin/env python

from pypb.session_commands_pb2 import Command_Register as Reg
from pypb.commands_pb2 import CommandContainer as Cmd

CMD_ID = 1

class Callback:
    def run(self, response):
        print "Got response: %s" % response

def build_reg():
    cmd = Cmd()
    sc = cmd.session_command.add()

    reg = sc.Extensions[Reg.ext]
    reg.user_name = "testUser"
    reg.email = "test@example.com"
    reg.password = "password"

    cmd.cmd_id = CMD_ID
    CMD_ID += 1
    return cmd

if __name__ == "__main__":
    print "Building registration command"
    r = build_reg()
    print "Attempting to register"
