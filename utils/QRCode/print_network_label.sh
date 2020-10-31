# Print a black and white PNG file on the first available Brother label printer
# Requires https://github.com/pklaus/brother_ql
# Source (.) this file to set bash completion and add the print function.
function print-png-label() {
    if [ -f "$1" ]; then
    
        # Change these to match your printer:
        local LBLPMODEL="QL-810W"
        local LBLPSIZE="12"

        # Query a bunch of times to allow the printer to wake up.
        for i in {1..5}; do
            local LBLPADDR=$(avahi-browse --resolve -t -p "_pdl-datastream._tcp" | grep ${LBLPMODEL} | egrep -m 1 "^=" | awk -F ";" '{print $8 ":" $9}')

            if [ ! "$LBLPADDR" == "" ]; then
                break
            fi
            sleep ${i}s
        done

        if [ "$LBLPADDR" == "" ]; then    
            echo "Error, printer not found using Avahi."
            return 2
        else
            brother_ql_print --backend network <( brother_ql_create --model ${LBLPMODEL} --label-size ${LBLPSIZE} "$1" ) tcp://${LBLPADDR}
        fi
    else  
        echo "Prints a black and white PNG to a networked Brother label printer."
        echo "See https://github.com/pklaus/brother_ql for details."
        return 1
    fi
}
complete -f -X '!*.@(bmp|dcx|gif|jpe|jpeg|jpg|png|ppm|tif|tiff)' print-png-label