tell application "Finder" to get folder of (path to me) as Unicode text

set workingDir to POSIX path of result

set myResult to do shell script "cd '" & workingDir & "'; pwd"



display dialog "About to convert all WAV files to 32768 Hz in the following folder

" & myResult


set myResult to do shell script "cd '" & workingDir & "'; /bin/sh ./convertTo32768.app/Contents/MacOS/convertAllWavs 32768 ."


set the clipboard to myResult

tell application "TextEdit"
	activate
	tell application "System Events" to keystroke "n" using command down
	tell application "System Events" to keystroke "v" using command down
end tell
