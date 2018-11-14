on run argv
  set image_name to item 1 of argv

  tell application "Finder"
  tell disk image_name

    -- wait for the image to finish mounting
    set open_attempts to 0
    repeat while open_attempts < 4
      try
        open
          delay 1
          set open_attempts to 5
        close
      on error errStr number errorNumber
        set open_attempts to open_attempts + 1
        delay 10
      end try
    end repeat
    delay 5

    -- open the image the first time and save a DS_Store with just
    -- background and icon setup
    open
      set current view of container window to icon view
      set theViewOptions to the icon view options of container window
      set background picture of theViewOptions to file ".background:background.png"
      set arrangement of theViewOptions to not arranged
      set icon size of theViewOptions to 64
      delay 5
    close

    -- next setup the position of the app and Applications symlink
    -- plus hide all the window decoration
    open
      update without registering applications
      tell container window
        set sidebar width to 0
        set statusbar visible to false
        set toolbar visible to false
        set the bounds to { 500, 300, 991, 620 }
        set position of item "Torrent File Editor.app" to { 400, 210 }
        set position of item "Applications" to { 97, 205 }
        set position of item ".DS_Store" to { 540, 10 }
        set position of item ".fseventsd" to { 540, 130 }
        set position of item ".background" to { 540, 230 }
        if exists item ".Trashes" then
          set position of item ".Trashes" to { 640, 10 }
        end if
      end tell
      update without registering applications
      delay 5
    close
  end tell
  delay 1
end tell
end run
