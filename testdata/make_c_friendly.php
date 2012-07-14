<?php 

if ($argc < 2) {
	echo "Wrong number of arguments, usage:".PHP_EOL;
	echo "php make_c_frindly.php input.json".PHP_EOL;
	exit(-1);
}
$filename = $argv[1];
$inFileHandle = fopen($filename, "r");
if ($inFileHandle) {
	$outFileHandle = fopen($filename.".out", "w");
	
	$contents = fread($inFileHandle, filesize($filename));
	$jsonContent = json_decode($contents, TRUE);
	
	$key = array_keys($jsonContent);
	$key = $key[0];
	foreach ($jsonContent[$key] as $entry) {
		$id = $entry["Id"];
		$outLine = $id."#".json_encode($entry)."\n";
		fputs($outFileHandle, $outLine);
	}
	fclose($outFileHandle);
	fclose($inFileHandle);
} else {
	echo "Input file not found".PHP_EOL;
	exit(-2);
}
exit(0);

?>