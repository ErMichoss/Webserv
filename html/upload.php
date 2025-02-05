<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Subir Archivo</title>
</head>
<body>
<?php
$directorio_destino = "uploads/";

if (!file_exists($directorio_destino)) {
    mkdir($directorio_destino, 0777, true);
}

if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_FILES['archivo'])) {
    $archivo = $_FILES['archivo'];
    if ($archivo['error'] !== UPLOAD_ERR_OK) {
        echo "Error en la carga del archivo. Código de error: " . $archivo['error'];
    } else {
        $nombreArchivo = basename($archivo['name']);
        $rutaDestino = $directorio_destino . $nombreArchivo;

        if (move_uploaded_file($archivo['tmp_name'], $rutaDestino)) {
            echo "Archivo subido con éxito: <a href='$rutaDestino'>$nombreArchivo</a>";
        } else {
            echo "Error al mover el archivo al destino.";
        }
    }
} else {
    echo "No se recibió ningún archivo.";
}

?>
    <h2>Subir un Archivo</h2>
    <form action="upload.php" method="post" enctype="multipart/form-data">
        <input type="file" name="archivo" required><br>
        <button type="submit">Subir</button>
    </form>
</body>
</html>
