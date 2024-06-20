<?php

require_once '../vendor/autoload.php';

$fafgearserver = new ztk\FafGear\Client();
$fafgearserver->setup( [
    'server_address' => '127.0.0.1'
]);
var_dump( $fafgearserver->connect() );
var_dump( $fafgearserver->submitSingleQuery('INSERT INTO test SET test=1') );

$fafgearserver->disconnect();
