 <?lua
    std.html()
 ?>
 <!DOCTYPE html>
<html>
<head>
<title>Page Title</title>
</head>
<body>

<h1>This is a Heading</h1>
<p>This is a paragraph.</p>
<ul>
<?lua

    local data = {k1 = "One", k2 = "two", k3 = "three"}
    for k,v in pairs(data) do
?>
    <li class = "<?=k..'1'?>" id = "<?=k..'2'?>" >
        <?lua echo(v) ?>
    </li>
<?lua
    end
    echo("end of \" %> lua")
?>
</ul>
</body>
</html> 