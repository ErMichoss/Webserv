<?php
$directorio_destino = "uploads/";

if (!file_exists($directorio_destino)) {
    mkdir($directorio_destino, 0777, true);
}

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_FILES['archivo'])) {
    $archivo = $_FILES['archivo'];
    $nombreArchivo = basename($archivo['name']);
    $rutaDestino = $directorio_destino . $nombreArchivo;

    if (move_uploaded_file($archivo['tmp_name'], $rutaDestino)) {
        echo "Archivo subido con éxito: <a href='$rutaDestino'>$nombreArchivo</a>";
    } else {
        echo "Error al subir el archivo.";
    }
} else {
    echo "No se recibió ningún archivo.";
}
?>
