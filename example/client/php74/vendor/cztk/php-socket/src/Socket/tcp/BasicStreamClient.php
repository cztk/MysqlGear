<?php


namespace ztk\Socket\tcp;


/**
 * @method user_socket_error_handler(array $array)
 */
class BasicStreamClient
{

    public int $socket_read_timeout_secs = 2;
    public int $socket_read_timeout_usecs = 0;

    public int $socket_send_timeout_secs = 2;
    public int $socket_send_timeout_usecs = 0;

    public string $socket_address;
    public int $socket_port = 1370;

    public int $debug_level = 5;
    public $user_socket_error_handler = null;

    private int $protocol_version = 1;
    //private Socket $connection_socket; # PHP8
    private $connection_socket = null;

    public function setup(array $config): void
    {
        if (isset($config['socket_read_timeout_secs'])) {
            $this->socket_read_timeout_secs = $config['socket_read_timeout_secs'];
        }
        if (isset($config['socket_read_timeout_usecs'])) {
            $this->socket_read_timeout_usecs = $config['socket_read_timeout_usecs'];
        }
        if (isset($config['socket_send_timeout_secs'])) {
            $this->socket_read_timeout_secs = $config['socket_send_timeout_secs'];
        }
        if (isset($config['socket_send_timeout_usecs'])) {
            $this->socket_read_timeout_usecs = $config['socket_send_timeout_usecs'];
        }
        if (isset($config['server_address'])) {
            $this->socket_address = $config['server_address'];
        }
        if (isset($config['server_port'])) {
            $this->socket_port = $config['server_port'];
        }
        if (isset($config['debug_level'])) {
            $this->debug_level = $config['debug_level'];
        }
        if (isset($config['user_socket_error_handler'])) {
            $this->user_socket_error_handler = $config['user_socket_error_handler'];
        }
    }

    //private function handleSocketError(?Socket $socket) : void { # PHP8

    public function connect(): bool
    {
        $this->disconnect();
        if (!$this->create()) {
            return false;
        }
        if (!$this->setSocketOptions()) {
            return false;
        }
        if (!@socket_connect($this->connection_socket, $this->socket_address, $this->socket_port)) {
            $this->handleSocketError($this->connection_socket);
            return false;
        }
        return true;
    }

    public function disconnect(): void
    {
        if ($this->connection_socket) {
            socket_close($this->connection_socket);
        }
        $this->connection_socket = null;
    }

    private function create(): bool
    {
        $domain = $this->identifyGearServerDomain($this->socket_address);

        $this->connection_socket = socket_create($domain, SOCK_STREAM, SOL_TCP);
        if (false === $this->connection_socket) {
            if ($this->debug_level < 1) {
                echo "socket error: unable to create a socket of type $domain\n";
            }
            $this->handleSocketError(null);
            return false;
        } else {
            return true;
        }
    }

    private function identifyGearServerDomain(string $address): int
    {
        if (filter_var($address, FILTER_VALIDATE_IP, FILTER_FLAG_IPV4)) {
            return AF_INET;
        }
        if (filter_var($address, FILTER_VALIDATE_IP, FILTER_FLAG_IPV6)) {
            return AF_INET6;
        }
        return AF_UNIX;
    }

    private function handleSocketError($socket): void
    {
        $errorcode = socket_last_error($socket);
        $errormsg = socket_strerror($errorcode);
        if ($this->debug_level < 1) {
            echo "socket error: " . $errorcode . " - " . $errormsg . "\n";
        }
        socket_clear_error($socket);
        if (null != $this->user_socket_error_handler) {
            $this->user_socket_error_handler(['errorcode' => $errorcode, 'errormsg' => $errormsg]);
        }
    }

    private function setSocketOptions(): bool
    {
        if (!socket_set_option($this->connection_socket, SOL_SOCKET, SO_SNDTIMEO, array('sec' => $this->socket_send_timeout_secs, 'usec' => $this->socket_send_timeout_usecs))) {
            $this->handleSocketError($this->connection_socket);
            return false;
        }
        if (!socket_set_option($this->connection_socket, SOL_SOCKET, SO_RCVTIMEO, array('sec' => $this->socket_read_timeout_secs, 'usec' => $this->socket_read_timeout_usecs))) {
            $this->handleSocketError($this->connection_socket);
            return false;
        }

        return true;
    }

    public function send(string $data, int $length, int $flags = 0): array
    {
        $result = ['sent' => false, 'bytes' => 0];
        $socket_send_result = socket_send($this->connection_socket, $data, $length, $flags);
        if (false !== $socket_send_result) {
            $result['sent'] = true;
            $result['bytes'] = $socket_send_result;
        } else {
            $this->handleSocketError($this->connection_socket);
        }
        return $result;
    }

    public function read(int $length, int $mode = PHP_BINARY_READ)
    {
        $result = ['read' => false, 'data' => "", 'bytes' => 0];
        $socket_read_result = socket_read($this->connection_socket, $length, $mode);
        if (false !== $socket_read_result) {
            $result['read'] = true;
            $result['data'] = $socket_read_result;
            $result['bytes'] = strlen($socket_read_result);
        } else {
            //TODO does it help?
            $this->handleSocketError($this->connection_socket);
        }
        return $result;
    }

}