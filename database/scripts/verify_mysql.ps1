[CmdletBinding()]
param(
    [string]$MySqlBin = 'D:\mysql-8.0.25-winx64\bin\mysql.exe',
    [string]$HostName = '127.0.0.1',
    [int]$Port = 3306,
    [string]$Username = 'root'
)

$ErrorActionPreference = 'Stop'
if (-not (Test-Path -LiteralPath $MySqlBin)) {
    throw "mysql.exe was not found: $MySqlBin"
}

if ([string]::IsNullOrEmpty($env:MYSQL_PWD)) {
    $securePassword = Read-Host 'MySQL password' -AsSecureString
    $passwordPointer = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($securePassword)
    try {
        $env:MYSQL_PWD = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($passwordPointer)
    }
    finally {
        [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($passwordPointer)
    }
}

$query = @'
SELECT
    (SELECT COUNT(*) FROM information_schema.tables WHERE table_schema = 'arcane_campus_online') AS table_count,
    (SELECT COUNT(*) FROM arcane_campus_online.users) AS user_count,
    (SELECT COUNT(*) FROM arcane_campus_online.rooms) AS room_count,
    (SELECT COUNT(*) FROM arcane_campus_online.npcs) AS npc_count;
'@

& $MySqlBin '--protocol=TCP' "--host=$HostName" "--port=$Port" "--user=$Username" '--default-character-set=utf8mb4' --batch --skip-column-names --execute=$query
if ($LASTEXITCODE -ne 0) {
    throw 'MySQL verification query failed.'
}
