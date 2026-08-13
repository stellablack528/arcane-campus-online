[CmdletBinding()]
param(
    [string]$MySqlBin = 'D:\mysql-8.0.25-winx64\bin\mysql.exe',
    [string]$HostName = '127.0.0.1',
    [int]$Port = 3306,
    [string]$Username = 'root'
)

$ErrorActionPreference = 'Stop'
$scriptRoot = Split-Path -Parent $PSScriptRoot
$sqlDirectory = Join-Path $scriptRoot 'sql'

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

$commonArguments = @('--protocol=TCP', "--host=$HostName", "--port=$Port", "--user=$Username", '--default-character-set=utf8mb4')
foreach ($scriptName in @('init_mysql.sql', 'migrate_v1.sql', 'seed_mysql.sql')) {
    $scriptPath = (Resolve-Path (Join-Path $sqlDirectory $scriptName)).Path.Replace('\', '/')
    & $MySqlBin @commonArguments --execute="source $scriptPath"
    if ($LASTEXITCODE -ne 0) {
        throw "MySQL import failed: $scriptName"
    }
}

Write-Output 'MySQL schema, migration marker, and seed data were imported successfully.'
