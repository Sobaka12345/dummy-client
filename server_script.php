<?php

$uploaddir = './files/';
$uploadfile = $uploaddir . basename($_FILES['uploaded']['name']);

if (move_uploaded_file($_FILES['uploaded']['tmp_name'], $uploadfile)) {
    chmod($uploadfile, 0666);
    $returnFile = "";
	if(isset($_GET['fileName'])) {
		$files = scandir($uploaddir);
		if(in_array($_GET['fileName'], $files))
			$returnFile = $uploaddir . $_GET['fileName'];
		else {
			echo "NO SUCH FILE";
			exit();
		}
	}
	else {
		$files = scandir($uploaddir);
		$files_flip = array_flip($files);
		unset($files_flip['.'], $files_flip['..']);
		$files = array_values(array_flip($files_flip));
		$returnFile = $uploaddir . $files[rand(0, count($files) - 1)];
	}
	header('Content-Disposition: attachment; filename=' . basename($returnFile));
	header('Content-Type: application/octet-stream');
	header('Content-Transfer-Encoding: Binary');
	header('Content-Length: '.filesize($returnFile));
    readfile($returnFile);
} else {
	echo "Fail: CAN'T UPLOAD THIS FILE. PLEASE CHECK THAT SERVER FILE DIR IS AVAILABLE";
    exit();
}

?>
