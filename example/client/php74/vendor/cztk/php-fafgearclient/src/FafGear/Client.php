<?php

namespace ztk\FafGear;

use ztk\Socket\tcp\BasicStreamClient;

class Client {

    public bool $isConnected = false;

    private BasicStreamClient $socket_connection;
    private int $protocol_version = 1;

    public function __construct() {
        $this->socket_connection = new BasicStreamClient();
    }

    private function errorHandler($args) {

    }

    private function send(string $data) : bool {
        if($this->isConnected) {
            $length = strlen($data);
            $send_result = $this->socket_connection->send($data, $length);
            return ($send_result['sent'] && $length == $send_result['bytes']);
        }
        return false;
    }

    private function read(int $bytes) : array {
        if($this->isConnected) {
            $read_result = $this->socket_connection->read($bytes);
            return $read_result;
        } else {
            return ['read' => false];
        }
    }

    public function setup(array $config) : void {

        $this->socket_connection->setup($config);
    }

    private function connect_socket() : bool {
        $this->isConnected = false;

        $connect_result = $this->socket_connection->connect();
        if($connect_result) {
            $this->isConnected = true;
            $header = pack("c", $this->protocol_version);
            if(!$this->send($header))  {
                $this->isConnected = false;
                $this->disconnect();
            }
        }

        return $this->isConnected;
    }

    public function connect() : bool {
        return $this->connect_socket();
    }

    public function disconnect(): void {
        if($this->isConnected) {
            $this->socket_connection->disconnect();
        }
    }

    public function getStatus() : array {
        $success = false;
        $statusData = "";

        $data = pack("cL", 1, 0);

        if($this->send($data)) {
            $statusDataLengthReq = $this->read(1);
            if($statusDataLengthReq['read']) {
                $statusDataLengthRaw = unpack("c", $statusDataLengthReq['data'] );
                $statusDataLength = $statusDataLengthRaw[1];
                $statusDataReq = $this->read($statusDataLength);
                if($statusDataReq['read']) {
                    $statusData = $statusDataReq['data'];
                    $success = true;
                }
            }
        }

        return ['success' => $success, 'data' => $statusData];
    }

    public function submitSingleQuery($query) : bool {
        $success = false;
        $query_strlen = strlen($query);
        $header = pack("cL", 3, $query_strlen);
        $data = $header.$query;
        if($this->send($data)) {
            $checkResult = $this->read(1);
            if($checkResult['read']) {
                if('1' == $checkResult['data']) {
                    $success = true;
                }
            }
        }
        return $success;
    }

}