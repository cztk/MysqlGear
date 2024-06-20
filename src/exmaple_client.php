<?php

    function poly_mysql_escape_string(string $unescaped_string): string
    {
        $replacementMap = [
            "\0" => "\\0",
            "\n" => "\\n",
            "\r" => "\\r",
            "\t" => "\\t",
            chr(26) => "\\Z",
            chr(8) => "\\b",
            '"' => '\"',
            "'" => "\'",
            '_' => "\_",
            "%" => "\%",
            '\\' => '\\\\'
        ];

        return \strtr($unescaped_string, $replacementMap);
    }


    $sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
    socket_set_option($sock, SOL_SOCKET, SO_SNDTIMEO, array('sec' => 2, 'usec' => 1));
    socket_connect($sock, '127.0.0.1', 1370);

// get status
$header = pack("cL", 1, 0);
$datalen = strlen($header);
socket_send($sock, $header, $datalen, 0);

$answer_length_raw = socket_read($sock, 1);
echo "answer_length_raw:\n";
var_dump( $answer_length_raw );

$answer_length = unpack("c", $answer_length_raw);
echo "answer_length\n";
var_dump($answer_length[1]);

$answer_data = socket_read($sock, $answer_length[1]);
echo "answer_data\n";
var_dump( $answer_data );



// send sql

for($i = 1; $i < 2; $i++ ) {
    $msg = '';
#    for($i = 0; $i < 2048; $i++) {
        $msg .= "INSERT INTO test SET test='". poly_mysql_escape_string($i) ."'";
#    }
    $len = strlen($msg);

    echo "\n".$len."\n";

    $header = pack("cL", 3, $len);

    $data = $header.$msg;
    $datalen = strlen($data);

    socket_send($sock, $data, $datalen, 0);

    var_dump( socket_read($sock, 1) );
}

    socket_close($sock);
