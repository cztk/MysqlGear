<?php

require_once '../vendor/autoload.php';

$fafgearserver = new ztk\FafGear\Client();
$fafgearserver->setup( [
    'server_address' => '127.0.0.1'
]);
var_dump( $fafgearserver->connect() );
$result = $fafgearserver->getStatus();
var_dump( $result );
if($result['success']) {
    $resultArray = explode(";", $result['data']);
    if(count($resultArray) >  10) {
        $i = 0;
        echo "Query queue size: " . $resultArray[$i++]."\n";

        echo "max database connections: " . $resultArray[$i++]."\n";
        echo "active database connections: " . $resultArray[$i++]."\n";

        echo "client threadpool count: " . $resultArray[$i++]."\n";
        echo "client threadpool running: " . $resultArray[$i++]."\n";
        echo "client threadpool queued: " . $resultArray[$i++]."\n";
        echo "client threadpool total: " . $resultArray[$i++]."\n";

        echo "database threadpool count: " . $resultArray[$i++]."\n";
        echo "database threadpool running: " . $resultArray[$i++]."\n";
        echo "database threadpool queued: " . $resultArray[$i++]."\n";
        echo "database threadpool total: " . $resultArray[$i++]."\n";

    }
}

$fafgearserver->disconnect();