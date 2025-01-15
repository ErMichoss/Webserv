<!DOCTYPE html>
<html lang="en">
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Formulario en PHP</title>
</head>
<body>
    <h1>Formulario en PHP</h1>


    <h2>Completa el formulario</h2>
    <form action="process.php" method="post" enctype="application/x-www-form-urlencoded">
        <label for="nombre">Nombre:</label>
        <input type="text" id="nombre" name="nombre" required>
        <br><br>
        <label for="edad">Edad:</label>
        <input type="text" id="edad" name="edad" required>
        <br><br>
        <button type="submit">Enviar</button>
    </form>

    <?php
	//Basicamente lo que esto hace es que pillo los datos que el cliente meta en las textbox y cunado se envie el boton enviar pues deberia actualizar la pagina
	// con los datos enviados.
        if ($_SERVER['REQUEST_METHOD'] == 'POST') {
            // Asegurarse de que los datos estén presentes
            $nombre = htmlspecialchars($_POST['nombre'] ?? '');
            $edad = htmlspecialchars($_POST['edad'] ?? '');

            echo "<h2>Datos recibidos:</h2><br>";
            echo "<p>Nombre: $nombre</p><br>";
            echo "<p>Edad: $edad</p><br>";
        }
    ?>
	
</body>
</html>
