param($filepath, $channelName, $channelNo, $serviceNo)
$tempPath = "d:/movie/p"

$TOOLS_PATH="C:\tools\DTV\CmCutScripts"

$lgdPath = Join-Path $TOOLS_PATH "lgd"

$LOGOG_PATH = Join-Path $TOOLS_PATH "logoGuillo_v208beta_r4\logoGuillo.exe"
$AVS2X_PATH = Join-Path $TOOLS_PATH "avs2pipemod.exe"
$AVSPLG_PATH = Join-Path $TOOLS_PATH "m2v_vfc\m2v.vfp"
$BONTSDEMUX_PATH = Join-Path $TOOLS_PATH "BonTsDemux\BonTsDemuxC.exe"
$TSSPLITTER_PATH = Join-Path $TOOLS_PATH "tssplitter.exe"
$MINMAXAUDIO_PATH = Join-Path $TOOLS_PATH "MinMaxAudio.jar"

$FINISHED_PATH="D:\movie\処理済み"

#
# プロセス起動して終了を待つ
#
function startProcess($file, $arguments) {
  echo "-----------------------------------------"
  echo "$file $arguments"
  echo "-----------------------------------------"

  $info = new-object System.Diagnostics.ProcessStartInfo
  $info.filename = $file
  $info.arguments = $arguments
  $info.useShellExecute = $false
  $info.WindowStyle = [System.Diagnostics.ProcessWindowStyle]::Minimized

  $process = [System.Diagnostics.Process]::start($info)
  $process.PriorityClass = [System.Diagnostics.ProcessPriorityClass]::Idle
  $process.waitForExit()
}

function demuxTS($file) {
  if (($file -ne $null) -and (test-path $file)) {
    startProcess "$BONTSDEMUX_PATH" "-bg -nogui -i `"$file`" -sound 0 -start -quit"
    move-item $file $FINISHED_PATH -force

    return $true
  }
  return $false
}

function split($file, $ch, $sv) {
  if ($sv -eq "141") {
    # BS日テレは番組情報遅い
    startProcess "$TSSPLITTER_PATH" "-EIT -ECM -1SEG -GOP -SEP -SEPA -OVL30,7,0,0,5 -CS `"$file`""
  } elseif ($sv -eq "211") {
    # BS11も番組情報遅め
    startProcess "$TSSPLITTER_PATH" "-EIT -ECM -1SEG -GOP -SEP -SEPA -OVL30,7,0,0,5 -CS `"$file`""
  } else {
    startProcess "$TSSPLITTER_PATH" "-EIT -ECM -1SEG -GOP -SEP -SEPA -OVL10,7,0,0,5 -CS `"$file`""
  }

  $path = convert-path $file
  $parent = split-path $path -Parent
  $leaf = split-path $path -Leaf

  $files = @()
  $files += join-path $parent ($leaf -replace ".ts$", "_HD*.ts") -resolve
  $files += join-path $parent ($leaf -replace ".ts$", "_SD*.ts") -resolve
  $files += join-path $parent ($leaf -replace ".ts$", "_CS*.ts") -resolve

  $result = @()
  foreach ($f in $files) {
    if (demuxTS $f) {
      echo "`"$f`""
      $result += ($f -replace ".ts$", ".m2v")
    }
  }

  return $result
}


function cmcut($files, $channelNo, $serviceNo) {
  $lgdFile = ""
  $autoTuneFile = ""

  switch -regex ($channelNo + '-' + $serviceNo) {
    "20-.*" {
      $lgdFile = Join-Path $lgdPath 'TokyoMX 1440x1080.lgd'
      $autoTuneFile = Join-Path $lgdPath 'TokyoMX 1440x1080.lgd.autoTune.param'
      break
    }
    "21-.*" {
      $lgdFile = Join-Path $lgdPath 'CX 1440x1080.lgd'
      $autoTuneFile = Join-Path $lgdPath 'CX 1440x1080.lgd.autoTune.param'
      break
    }
    "22-.*" {
      $lgdFile = Join-Path $lgdPath 'TBS 1440x1080.lgd'
      $autoTuneFile = Join-Path $lgdPath 'TBS 1440x1080.lgd.autoTune.param'
      break
    }
    "23-.*" {
      $lgdFile = Join-Path $lgdPath 'TvTokyo 1440x1080.lgd'
      $autoTuneFile = Join-Path $lgdPath 'TvTokyo 1440x1080.lgd.autoTune.param'
      break
    }
    "25-.*" {
      $lgdFile = Join-Path $lgdPath 'ZeroTV 1440x1080.lgd'
      $autoTuneFile = Join-Path $lgdPath 'ZeroTV 1440x1080.lgd.autoTune.param'
      break
    }
    "^.*-141" {
      $lgdFile = Join-Path $lgdPath 'BS-NTV_1920x1080.lgd'
      $autoTuneFile = Join-Path $lgdPath 'BS-NTV_1920x1080.lgd.autoTune.param'
      break
    }
    "^.*-161" {
      $lgdFile = Join-Path $lgdPath 'BS-TBS 1920x1080.lgd'
      $autoTuneFile = Join-Path $lgdPath 'BS-TBS 1920x1080.lgd.autoTune.param'
      break
    }
    "^.*-181" {
      $lgdFile = Join-Path $lgdPath 'BSCX 1920x1080.lgd'
      $autoTuneFile = Join-Path $lgdPath 'BSCX 1920x1080.lgd.autoTune.param'
      break
    }
    "^.*-211" {
      $lgdFile = Join-Path $lgdPath 'BS11 1920x1080.lgd'
      $autoTuneFile = Join-Path $lgdPath 'BS11 1920x1080.lgd.autoTune.param'
      break
    }
    "^.*-222" {
      $lgdFile = Join-Path $lgdPath 'TwellV 1440x1080.lgd'
      $autoTuneFile = Join-Path $lgdPath 'TwellV 1440x1080.lgd.autoTune.param'
      break
    }
    "^.*-236" {
      $lgdFile = Join-Path $lgdPath 'animax 1440x1080.lgd'
      $autoTuneFile = Join-Path $lgdPath 'animax 1440x1080.lgd.autoTune.param'
      break
    }
  }

  if ($lgdFile -ne "") {
    foreach ($f in $files) {
      if ($f -ne "") {
        # 無音領域ファイル作成
        $wavFileName = $f -replace ".m2v$", ".wav"
        $silentFileName = $f -replace ".m2v$", ".silent"
        startProcess "java" "-jar `"$MINMAXAUDIO_PATH`" `"$wavFileName`" `"$silentFileName`""

        # logogillo起動
        startProcess `
            "$LOGOG_PATH" `
            "-video `"$f`" `
             -lgd `"$lgdFile`" `
             -avs2x `"$AVS2X_PATH`" `
             -avsPlg `"$AVSPLG_PATH`" `
             -prm `"$autoTuneFile`" `
             -useSil `"$silentFileName`" `
             -out `"$f.txt`" -outFmt keyF"
      }
    }
  }
}


mv $filepath $tempPath

"{0}, {1}, {2}, {3}" -F $filepath, $channelName, $channelNo, $serviceNo | Out-File d:/movie/01.output.txt -append

if ($filepath -ne "") {
  $fileName = Split-Path $filepath -Leaf
  $toFileName = Join-Path $tempPath $fileName

  echo "InputFileName : ${fileName}"
  echo "OutputFileName : ${toFileName}"

  # ファイル分割
  $files = split $toFileName $channelNo $serviceNo

  # CMカット
  cmcut $files $channelNo $serviceNo
}

