import socket
import argparse
import time
from random import randrange
import os

BUFF_SIZE = 2041


def if_drop(drop_val):
    if randrange(start = 1, stop = 11) <= drop_val:
        return True
    return False


def print_stats(data_pkts_dropped, data_pkts_recvd, data_pkts_sent, ack_pkts_dropped, ack_pkts_recvd, ack_pkts_sent, last_pkt_dropped):
    os.system('cls' if os.name == 'nt' else 'clear')
    print(f"""***********************************************************************
    Data Packets Received: {data_pkts_recvd}, Data Packets Dropped: {data_pkts_dropped}, Data Packets Sent: {data_pkts_sent} \n
    ACK Packets Received: {ack_pkts_recvd}, ACK Packets Dropped: {ack_pkts_dropped}, ACK Packets Sent: {ack_pkts_sent} \n
    Latest Dropped Packet Type: {last_pkt_dropped} \n
    ************************************************************************""")


def start(client, server, ack_pkt_drop_val, data_pkt_drop_val):
    last_pkt_dropped = "NONE"
    data_pkts_dropped = 0
    data_pkts_recvd = 0
    data_pkts_sent = 0
    ack_pkts_dropped = 0
    ack_pkts_recvd = 0
    ack_pkts_sent = 0
    while True:
        message = client.recv(BUFF_SIZE)
        data_pkts_recvd += 1
        while True:
            if not if_drop(data_pkt_drop_val):
                break
            # print("[data pkt dropped]")
            data_pkts_dropped += 1
            last_pkt_dropped = "DATA"
            message = client.recv(BUFF_SIZE)
            data_pkts_recvd += 1

        # print("[sending data pkt to server]")
        server.sendall(message)
        data_pkts_sent += 1
        message = server.recv(BUFF_SIZE)
        ack_pkts_recvd += 1

        if if_drop(ack_pkt_drop_val):
            ack_pkts_dropped += 1
            last_pkt_dropped = "ACK"
            # print("[ack pkt dropped]")
        else:
            ack_pkts_sent += 1
            client.sendall(message)

        print_stats(data_pkts_dropped, data_pkts_recvd, data_pkts_sent, ack_pkts_dropped, ack_pkts_recvd, ack_pkts_sent, last_pkt_dropped)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-p", "--conn_port", help="port that proxy connects", type=int)
    parser.add_argument("-l", "--listn_port", help="port that proxy listens", type=int)
    parser.add_argument("-e", "--proxy_ip", help="IP address for proxy")
    parser.add_argument("-s", "--server_ip", help="IP address of server")
    parser.add_argument("-a", "--ack_drop_val", help="ack drop percentage [1, 10]", type=int)
    parser.add_argument("-d", "--data_drop_val", help="data drop percentage [1, 10]", type=int)


    args = parser.parse_args()

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.connect((args.server_ip, args.conn_port))

    client_listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_listener.bind((args.proxy_ip, args.listn_port))
    client_listener.listen()

    client, addr = client_listener.accept()

    print(f"New session started, client connected: {addr[0]}")
    start(client, server, args.ack_drop_val, args.data_drop_val)


if __name__ == '__main__':
    main()