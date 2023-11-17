# Define a function to download and unzip a file
function(download_and_unzip target_dir download_url)
    # Extract the filename from the URL
    string(REGEX MATCH "[^/]+$" zip_file "${download_url}")

    # Set the full path to the zip file
    set(zip_path "${target_dir}/${zip_file}")

    # Check if the zip file already exists
    if(NOT EXISTS "${zip_path}")
        message(STATUS "Downloading ${zip_file} from ${download_url}...")

        # Download the zip file
        file(DOWNLOAD ${download_url} ${zip_path}
             STATUS download_status
             LOG download_log)

        # Check if the download was successful
        list(GET download_status 0 status_code)
        if(status_code EQUAL 0)
            message(STATUS "Download of ${zip_file} completed.")
        else()
            message(FATAL_ERROR "Error downloading ${zip_file}: ${download_log}")
        endif()

        # Unzip the downloaded file
        message(STATUS "Unzipping ${zip_file}...")
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${zip_path}
                        WORKING_DIRECTORY ${target_dir}
                        RESULT_VARIABLE unzip_result)
        
        if(NOT unzip_result EQUAL "0")
            message(FATAL_ERROR "Error unzipping ${zip_file}")
        endif()

        # Optionally, delete the zip file after extraction
        # file(REMOVE ${zip_path})
    else()
        message(STATUS "${zip_file} already exists. Skipping download.")
    endif()
endfunction()