/*
    Copyright (c) 2007-2016 Contributors as noted in the AUTHORS file

    This file is part of libzmq, the ZeroMQ core engine in C++.

    libzmq is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    As a special exception, the Contributors give you permission to link
    this library with independent modules to produce an executable,
    regardless of the license terms of these independent modules, and to
    copy and distribute the resulting executable under terms of your choice,
    provided that you also meet, for each linked independent module, the
    terms and conditions of the license of that module. An independent
    module is a module which is not derived from or based on this library.
    If you modify this library, you must extend this exception to your
    version of the library.

    libzmq is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>

#include "precompiled.hpp"
#include "mailbox.hpp"
#include "command.hpp"
#include "err.hpp"
#include "object.hpp"


zmq::mailbox_t::mailbox_t ()
{
    //  Get the pipe into passive state. That way, if the users starts by
    //  polling on the associated file descriptor it will get woken up when
    //  new command is posted.
    const bool ok = _cpipe.check_read ();
    zmq_assert (!ok);
    _active = false;
}

zmq::mailbox_t::~mailbox_t ()
{
    //  TODO: Retrieve and deallocate commands inside the _cpipe.

    // Work around problem that other threads might still be in our
    // send() method, by waiting on the mutex before disappearing.
    _sync.lock ();
    _sync.unlock ();
}

zmq::fd_t zmq::mailbox_t::get_fd () const
{
    return _signaler.get_fd ();
}


const char* cmd_name(int cmd_type)
{
  switch(cmd_type)
  {
    case 0: return "stop";
    case 1: return "plug";
    case 2: return "attach";
    case 3: return "bind";
    case 4: return "activate_read";
    case 5: return "activate_write";
    case 6: return "hiccup";
    case 7: return "pipe_term";
    case 8: return "pipe_term_ack";
    case 9: return "pipe_hwm";
    case 10: return "term_req";
    case 11: return "term";
    case 12: return "term_ack";
    case 13: return "term_endpoint";
    case 14: return "reap";
    case 15: return "reaped";
    case 16: return "inproc_connected";
    case 17: return "conn_failed";
    case 18: return "pipe_peer_stats";
    case 19: return "pipe_stats_publish";
    case 20: return "done";
    default: return "other";
  }
}

void zmq::mailbox_t::send (const command_t &cmd_)
{
    std::cout << "Sending internal command: " << cmd_name(cmd_.type) << " to " << cmd_.destination->get_tid() << "\n";
    _sync.lock ();
    _cpipe.write (cmd_, false);
    const bool ok = _cpipe.flush ();
    _sync.unlock ();
    if (!ok)
        _signaler.send ();
}

int zmq::mailbox_t::recv (command_t *cmd_, int timeout_)
{
    //  Try to get the command straight away.
    if (_active) {
        if (_cpipe.read (cmd_))
            return 0;

        //  If there are no more commands available, switch into passive state.
        _active = false;
    }

    //  Wait for signal from the command sender.
    int rc = _signaler.wait (timeout_);
    if (rc == -1) {
        errno_assert (errno == EAGAIN || errno == EINTR);
        return -1;
    }

    //  Receive the signal.
    rc = _signaler.recv_failable ();
    if (rc == -1) {
        errno_assert (errno == EAGAIN);
        return -1;
    }

    //  Switch into active state.
    _active = true;

    //  Get a command.
    const bool ok = _cpipe.read (cmd_);
    std::cout << "Received internal command: " << cmd_name(cmd_->type) <<  " to " << cmd_->destination->get_tid() << "\n";
    zmq_assert (ok);
    return 0;
}

bool zmq::mailbox_t::valid () const
{
    return _signaler.valid ();
}
