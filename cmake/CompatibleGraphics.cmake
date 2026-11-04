option(CHI_WINDOWS_SOFTWARE_GRAPHICS "Default to CPU/GDI on Windows" ON)
# JUCE 7.0.12 already creates Windows peers with GDI and NativeImageType with
# SoftwarePixelData. Check the actual source instead of patching unrelated JUCE 9
# snippets or modifying a user-supplied framework checkout.
if(WIN32 AND CHI_WINDOWS_SOFTWARE_GRAPHICS)
    file(READ "${JUCE_SOURCE_DIR}/modules/juce_gui_basics/native/juce_Windowing_windows.cpp" peer_source)
    file(READ "${JUCE_SOURCE_DIR}/modules/juce_graphics/images/juce_Image.cpp" image_source)
    string(REPLACE "\r\n" "\n" peer_source "${peer_source}")
    string(REPLACE "\r\n" "\n" image_source "${image_source}")
    if(NOT peer_source MATCHES "currentRenderingEngine \\(softwareRenderingEngine\\)" OR
       NOT image_source MATCHES "return new SoftwarePixelData \\(format, width, height, clearImage\\);")
        message(FATAL_ERROR "Compatible graphics expects the pinned JUCE 7 GDI/CPU factories. Audit any framework upgrade before building.")
    endif()
endif()
file(WRITE "${CMAKE_BINARY_DIR}/compatible-graphics.txt"
    "JUCE: 7.0.12 (4f43011b96eb0636104cb3e433894cda98243626)\nCHI_WINDOWS_SOFTWARE_GRAPHICS=${CHI_WINDOWS_SOFTWARE_GRAPHICS}\nWindows upstream initial peer: GDI (0); NativeImageType: CPU\nDirect2D is an explicit editor preference; no JUCE checkout modified.\n")
