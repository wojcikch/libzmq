# break zmq_msg_recv
# break s_recvmsg(zmq::socket_base_t*, zmq_msg_t*, int)
# break zmq::socket_base_t::recv(zmq::msg_t*, int)
# break zmq::socket_base_t::process_commands(int, bool)
# break zmq::signaler_t::wait(int) const

# Process a single command from the signaler (internal comm)
# break zmq::object_t::process_command(zmq::command_t const&) 
# break zmq::mailbox_t::send

# TCP communication
# break zmq::zmtp_engine_t::handshake()
# break zmq::zmtp_engine_t::receive_greeting()
# break zmq::zmtp_engine_t::receive_greeting_versioned()
# break stream_engine_base.cpp:285 if rc > 0
# break zmq::tcp_read(int, void*, unsigned long)
break zmq::tcp_write if size_ == 26

# Handshake
# break zmq::stream_engine_base_t::restart_output()
break zmq::stream_engine_base_t::process_handshake_command(zmq::msg_t*)
break zmq::mechanism_t::parse_metadata(unsigned char const*, unsigned long, bool)

# break zmq::socket_base_t::bind(char const*)
# break src/epoll.cpp:191 " This is the loop that accepts all events
