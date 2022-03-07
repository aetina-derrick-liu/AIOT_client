# -*- coding: UTF-8 -*-
# This file is part of the jetson_stats package (https://github.com/rbonghi/jetson_stats or http://rnext.it).
# Copyright (c) 2019 Raffaello Bonghi.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

import re
import os
from random import choice
from string import ascii_letters
from base64 import b64encode
# Launch command
import subprocess as sp
# Logging
import logging
# Socket and IP information
import socket
import fcntl
import struct
import array
from .exceptions import JtopException
# Load Author
AUTH_RE = re.compile(r""".*__author__ = ["'](.*?)['"]""", re.S)
# Create logger
logger = logging.getLogger(__name__)


class Board:

    def __init__(self):
        self.info = {}
        self.hardware = {}
        self.libraries = {}
        self._board = {'info': self.info, 'hardware': self.hardware, 'libraries': self.libraries}

    def _update_libraries(self, libraries):
        self.libraries = libraries
        self._board = {'info': self.info, 'hardware': self.hardware, 'libraries': self.libraries}

    def _update_init(self, init):
        self.info = init['info']
        self.hardware = init['hardware']
        self._board = {'info': self.info, 'hardware': self.hardware, 'libraries': self.libraries}

    def items(self):
        return self._board.items()

    def get(self, name, value=None):
        if name in self._board:
            return self._board[name]
        else:
            return value

    def __getitem__(self, name):
        return self._board[name]

    def __iter__(self):
        return iter(self._board)

    def __next__(self):
        return next(self._board)

    def __len__(self):
        return len(self._board)

    def __repr__(self):
        return str(self._board)


def locate_commands(name, commands):
    for cmd in commands:
        if os.path.exists(cmd):
            logger.info("{name} loaded on {cmd}".format(name=name, cmd=cmd))
            return cmd
    raise JtopException("{name} is not availabe on this board".format(name=name))


def import_os_variables(SOURCE, PATTERN):
    if os.path.isfile(SOURCE):
        logger.debug("Open source file {source}".format(source=SOURCE))
        proc = sp.Popen(['bash', '-c', 'source {source} && env'.format(source=SOURCE)], stdout=sp.PIPE, stderr=sp.PIPE)
        # Load variables
        source_env = {}
        for tup in map(lambda s: s.decode("utf-8").strip().split('=', 1), proc.stdout):
            name = tup[0].strip()
            value = tup[1].strip()
            if PATTERN in name:
                source_env[name] = value
        return source_env
    else:
        logger.error("File does not exist")
        return {}


def get_var(MATCH_RE):
    """
    Show the version of this package

    :return: Version number
    :rtype: string
    """
    # Load version package
    with open(os.path.join(os.path.abspath(os.path.dirname(__file__)), '../', "__init__.py")) as fp:
        match = MATCH_RE.match(fp.read())
        value = match.group(1) if match else ''.join(choice(ascii_letters) for i in range(16))
    return value


def get_uptime():
    """ Read uptime system
        http://planzero.org/blog/2012/01/26/system_uptime_in_python,_a_better_way
    """
    with open('/proc/uptime', 'r') as f:
        uptime_seconds = float(f.readline().split()[0])
    return uptime_seconds


def status_disk(folder="/var/"):
    disk = os.statvfs(folder)
    # Evaluate the total space in GB
    totalSpace = float(disk.f_bsize * disk.f_blocks) / 1024 / 1024 / 1024
    # Evaluate total used space in GB
    totalUsedSpace = float(disk.f_bsize * (disk.f_blocks - disk.f_bfree)) / 1024 / 1024 / 1024
    # Evaluate total available space in GB
    totalAvailSpace = float(disk.f_bsize * disk.f_bfree) / 1024 / 1024 / 1024
    # Evaluate total non super-user space in GB
    totalAvailSpaceNonRoot = float(disk.f_bsize * disk.f_bavail) / 1024 / 1024 / 1024
    return {'total': totalSpace,
            'used': totalUsedSpace,
            'available': totalAvailSpace,
            'available_no_root': totalAvailSpaceNonRoot
            }


def get_local_interfaces():
    """ Returns a dictionary of name:ip key value pairs.
        - Reference:
           * http://code.activestate.com/recipes/439093/#c1
           * https://gist.github.com/pklaus/289646
    """
    # Max possible bytes for interface result.  Will truncate if more than 4096 characters to describe interfaces.
    MAX_BYTES = 4096
    # We're going to make a blank byte array to operate on.  This is our fill char.
    FILL_CHAR = b'\0'
    # Command defined in ioctl.h for the system operation for get iface list
    # Defined at https://code.woboq.org/qt5/include/bits/ioctls.h.html under
    # /* Socket configuration controls. */ section.
    SIOCGIFCONF = 0x8912
    # Read hostname
    hostname = socket.gethostname()
    # Make a dgram socket to use as our file descriptor that we'll operate on.
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Make a byte array with our fill character.
    names = array.array('B', MAX_BYTES * FILL_CHAR)
    # Get the address of our names byte array for use in our struct.
    names_address, names_length = names.buffer_info()
    # Create a mutable byte buffer to store the data in
    mutable_byte_buffer = struct.pack('iL', MAX_BYTES, names_address)
    # mutate our mutable_byte_buffer with the results of get_iface_list.
    # NOTE: mutated_byte_buffer is just a reference to mutable_byte_buffer - for the sake of clarity we've defined them as
    # separate variables, however they are the same address space - that's how fcntl.ioctl() works since the mutate_flag=True
    # by default.
    mutated_byte_buffer = fcntl.ioctl(sock.fileno(), SIOCGIFCONF, mutable_byte_buffer)
    # Get our max_bytes of our mutated byte buffer that points to the names variable address space.
    max_bytes_out, names_address_out = struct.unpack('iL', mutated_byte_buffer)
    # Convert names to a bytes array - keep in mind we've mutated the names array, so now our bytes out should represent
    # the bytes results of the get iface list ioctl command.
    namestr = names.tostring()
    # Each entry is 40 bytes long.  The first 16 bytes are the name string.
    # the 20-24th bytes are IP address octet strings in byte form - one for each byte.
    # Don't know what 17-19 are, or bytes 25:40.
    ip_dict = {}
    for i in range(0, max_bytes_out, 40):
        name = namestr[i: i + 16].split(FILL_CHAR, 1)[0]
        name = name.decode('utf-8')
        ip_bytes = namestr[i + 20:i + 24]
        full_addr = []
        for netaddr in ip_bytes:
            if isinstance(netaddr, int):
                full_addr.append(str(netaddr))
            elif isinstance(netaddr, str):
                full_addr.append(str(ord(netaddr)))
        ip_dict[name] = '.'.join(full_addr)
    # Remove loopback interface is in list
    if 'lo' in ip_dict:
        del ip_dict['lo']
    return {"hostname": hostname, "interfaces": ip_dict}


def get_key():
    return str(b64encode(get_var(AUTH_RE).encode("utf-8")))
# EOF
