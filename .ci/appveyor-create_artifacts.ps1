$exe = dir -name *.exe
$new_name = $exe.Replace(".exe", "-${env:target_arch}.exe")
Push-AppveyorArtifact $exe -FileName $new_name
$cmake_name = $exe.Replace(".exe", "-${env:target_arch}.cmake.txt")
Push-AppveyorArtifact CMakeCache.txt -FileName $cmake_name
$json = New-Object PSObject
(New-Object PSObject | Add-Member -PassThru NoteProperty bin $new_name | Add-Member -PassThru NoteProperty cmake $cmake_name | Add-Member -PassThru NoteProperty commit $env:APPVEYOR_REPO_COMMIT) | ConvertTo-JSON | Out-File -FilePath "latest-$env:target_arch" -Encoding ASCII
Push-AppveyorArtifact "latest-$env:target_arch"
$version = $matches['content']
