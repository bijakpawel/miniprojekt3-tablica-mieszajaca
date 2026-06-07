param(
    [string]$InputDir = (Join-Path $PSScriptRoot 'wyniki'),
    [string]$OutputDir = (Join-Path $PSScriptRoot 'wykresy')
)

Add-Type -AssemblyName System.Windows.Forms.DataVisualization

function Get-Measurements {
    param(
        [string]$Path
    )

    Import-Csv -Path $Path | ForEach-Object {
        [pscustomobject]@{
            Size   = [int]$_.rozmiar
            Insert = [double]$_.insert
            Remove = [double]$_.remove
        }
    }
}

function New-PerformanceChart {
    param(
        [string]$Title,
        [string]$YAxisTitle,
        [string]$OutputPath,
        [array]$SeriesData,
        [string]$ValueProperty
    )

    $chart = New-Object System.Windows.Forms.DataVisualization.Charting.Chart
    $chart.Width = 1600
    $chart.Height = 900
    $chart.BackColor = [System.Drawing.Color]::White

    $area = New-Object System.Windows.Forms.DataVisualization.Charting.ChartArea 'Main'
    $area.BackColor = [System.Drawing.Color]::FromArgb(250, 250, 250)
    $area.AxisX.Title = 'Rozmiar zbioru'
    $area.AxisY.Title = $YAxisTitle
    $area.AxisX.MajorGrid.LineColor = [System.Drawing.Color]::FromArgb(220, 220, 220)
    $area.AxisY.MajorGrid.LineColor = [System.Drawing.Color]::FromArgb(220, 220, 220)
    $area.AxisX.LabelStyle.Angle = -45
    $area.AxisX.Interval = 1
    $area.AxisX.IsMarginVisible = $true
    $area.AxisX.TitleFont = New-Object System.Drawing.Font('Segoe UI', 13, [System.Drawing.FontStyle]::Regular)
    $area.AxisY.TitleFont = New-Object System.Drawing.Font('Segoe UI', 13, [System.Drawing.FontStyle]::Regular)
    $area.AxisX.LabelStyle.Font = New-Object System.Drawing.Font('Segoe UI', 10)
    $area.AxisY.LabelStyle.Font = New-Object System.Drawing.Font('Segoe UI', 10)
    $chart.ChartAreas.Add($area)

    $legend = New-Object System.Windows.Forms.DataVisualization.Charting.Legend 'Legend'
    $legend.Docking = [System.Windows.Forms.DataVisualization.Charting.Docking]::Top
    $legend.Alignment = [System.Drawing.StringAlignment]::Center
    $legend.Font = New-Object System.Drawing.Font('Segoe UI', 11)
    $chart.Legends.Add($legend)

    $chart.Titles.Add($Title) | Out-Null
    $chart.Titles[0].Font = New-Object System.Drawing.Font('Segoe UI Semibold', 18)
    $chart.Titles[0].ForeColor = [System.Drawing.Color]::FromArgb(35, 35, 35)
    $chart.Titles[0].Alignment = [System.Drawing.ContentAlignment]::TopCenter

    $seriesPalette = @(
        [System.Drawing.Color]::FromArgb(44, 62, 80),
        [System.Drawing.Color]::FromArgb(52, 152, 219),
        [System.Drawing.Color]::FromArgb(39, 174, 96)
    )

    for ($i = 0; $i -lt $SeriesData.Count; $i++) {
        $item = $SeriesData[$i]
        $series = New-Object System.Windows.Forms.DataVisualization.Charting.Series $item.Name
        $series.ChartType = [System.Windows.Forms.DataVisualization.Charting.SeriesChartType]::Line
        $series.BorderWidth = 3
        $series.MarkerStyle = [System.Windows.Forms.DataVisualization.Charting.MarkerStyle]::Circle
        $series.MarkerSize = 7
        $series.Color = $seriesPalette[$i % $seriesPalette.Count]
        $series.IsValueShownAsLabel = $false
        $series.ChartArea = 'Main'

        foreach ($point in $item.Data) {
            $xValue = [string]$point.Size
            $yValue = [double]$point.$ValueProperty
            [void]$series.Points.AddXY($xValue, $yValue)
        }

        $chart.Series.Add($series)
    }

    $directory = Split-Path -Parent $OutputPath
    if (-not (Test-Path $directory)) {
        New-Item -ItemType Directory -Path $directory | Out-Null
    }

    $chart.SaveImage($OutputPath, [System.Windows.Forms.DataVisualization.Charting.ChartImageFormat]::Png)
    $chart.Dispose()
}

$chaining = Get-Measurements -Path (Join-Path $InputDir 'pomiary_chaining.csv')
$linear = Get-Measurements -Path (Join-Path $InputDir 'pomiary_linear.csv')
$avl = Get-Measurements -Path (Join-Path $InputDir 'pomiary_avl.csv')

$series = @(
    @{ Name = 'Lancuchowanie'; Data = $chaining },
    @{ Name = 'Adresowanie otwarte'; Data = $linear },
    @{ Name = 'Koszyk AVL'; Data = $avl }
)

New-PerformanceChart `
    -Title 'Miniprojekt 3: czas operacji insert' `
    -YAxisTitle 'Czas [ns]' `
    -OutputPath (Join-Path $OutputDir 'wykres_insert_final.png') `
    -SeriesData $series `
    -ValueProperty 'Insert'

New-PerformanceChart `
    -Title 'Miniprojekt 3: czas operacji remove' `
    -YAxisTitle 'Czas [ns]' `
    -OutputPath (Join-Path $OutputDir 'wykres_remove_final.png') `
    -SeriesData $series `
    -ValueProperty 'Remove'

Write-Host "Wykresy zapisano w: $OutputDir"