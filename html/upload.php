<?php
if (!isset($_FILES["myfile"])) {
    die("Error: No se ha seleccionado ningún archivo.");
}
	$target_dir = "/home/nicgonza/webserv-entrega/html/uploads/";
	$target_file = $target_dir . basename($_FILES["myfile"]["name"]);
	$uploadOk = 1;
	$imageFileType = strtolower(pathinfo($target_file,PATHINFO_EXTENSION));

	// Check if file already exists
	if (file_exists($target_file)) {
	  echo "Sorry, file already exists.".$target_file;
	  $uploadOk = 0;
	}

	// Check if $uploadOk is set to 0 by an error
	if ($uploadOk == 0) {
	  echo "Sorry, your file was not uploaded.";
	// if everything is ok, try to upload file
	} else {
	  if (move_uploaded_file($_FILES["myfile"]["tmp_name"], $target_file)) {
	    echo "The file ". htmlspecialchars( basename( $_FILES["myfile"]["name"])). " has been uploaded.";
	  } else {
	    echo "Sorry, there was an error uploading your file.". $_FILES["myfile"]['error'];
	  }
	}
?>