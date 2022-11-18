#
# Example python script to generate a BOM from a KiCad generic netlist
#
# Example: Sorted and Grouped CSV BOM
#

"""
    @package
    Output: CSV (comma-separated)
    Grouped By: Value, Footprint
    Sorted By: Ref
    Fields: Ref, Qnty, Value, Cmp name, Footprint, Description, Vendor

    Command line:
    python "pathToFile/bom_csv_grouped_by_value_with_fp.py" "%I" "%O.csv"
"""

# Import the KiCad python helper module and the csv formatter
import kicad_netlist_reader
import kicad_utils
import csv
import sys
import mouser
from mouser.cli import mouser_cli


def ask_price(part_number_list):
    operation = "partnumber"
    API_KEYS_FILE = "/home/nikolasf/Dokumente/01_git/30_Python/BOM_KiCAD/mouser_api_keys.yaml"

    prices = []

    for part_number in part_number_list:
        print(f"{part_number = }")
        request = mouser.cli.MouserPartSearchRequest(operation, API_KEYS_FILE, '', "", part_number)
        search = request.part_search(part_number)

        if search:
            #request.print_clean_response()
            try:

                clean_response = request.get_clean_response()
                prices.append(clean_response["PriceBreaks"][0]["Price"])

                print(clean_response)
            except:
                prices.append(' ')

    return prices


def ask_price_availability(part_number):
    operation = "partnumber"
    API_KEYS_FILE = "/home/nikolasf/Dokumente/01_git/30_Python/BOM_KiCAD/mouser_api_keys.yaml"

    prices = []

    request = mouser.cli.MouserPartSearchRequest(operation, API_KEYS_FILE, '', "", part_number)
    search = request.part_search(part_number)

    if search:
        #request.print_clean_response()
        try:

            clean_response = request.get_clean_response()
            price = clean_response["PriceBreaks"][0]["Price"]
            availability = clean_response["Availability"]

            print(clean_response)
        except:
            price = 'unknown'
            availability = "unknown"

    else:
        price = "unknown"
        availability = "unknown"


    return price, availability

# A helper function to filter/convert a string read in netlist
#currently: do nothing
def fromNetlistText( aText ):
    return aText

# Generate an instance of a generic netlist, and load the netlist tree from
# the command line option. If the file doesn't exist, execution will stop
net = kicad_netlist_reader.netlist(sys.argv[1])

# Open a file to write to, if the file cannot be opened output to stdout
# instead
try:
    f = kicad_utils.open_file_writeUTF8(sys.argv[2], 'w')
except IOError:
    e = "Can't open output file for writing: " + sys.argv[2]
    print(__file__, ":", e, sys.stderr)
    f = sys.stdout

# Create a new csv writer object to use as the output formatter
out = csv.writer(f, lineterminator='\n', delimiter=',', quotechar='\"', quoting=csv.QUOTE_ALL)

# Output a set of rows for a header providing general information
out.writerow(['Source:', net.getSource()])
out.writerow(['Date:', net.getDate()])
out.writerow(['Tool:', net.getTool()])
out.writerow( ['Generator:', sys.argv[0]] )
out.writerow(['Component Count:', len(net.components)])
out.writerow(['Ref', 'Qnty', 'Mouser', 'Value', 'Cmp name', 'Footprint'])


# Get all of the components in groups of matching parts + values
# (see ky_generic_netlist_reader.py)
grouped = net.groupComponents()

total_cost = 0

# Output all of the component information
for group in grouped:
    refs = ""

    # Add the reference of every component in the group and keep a reference
    # to the component so that the other data can be filled in once per group
    for component in group:
        refs += fromNetlistText( component.getRef() ) + ", "
        c = component

    mouser_part_number = fromNetlistText(c.getField("mouser#"))
    print(f"{mouser_part_number = }")
    if mouser_part_number == "":
        mouser_price = 0
        mouser_availability = 0
    else:
        mouser_price, mouser_availability = ask_price_availability(mouser_part_number)
        try:
            mouser_price = mouser_price.replace(" €", "")
            mouser_price = mouser_price.replace(",", ".")
            mouser_price_float = float(mouser_price)
        except:
            mouser_price_float = 0
            mouser_price = "Nicht vorhanden"
        total_cost += len(group) * mouser_price_float
        print(f"{total_cost = }")


    # Fill in the component groups common data
    out.writerow([refs, len(group),
        fromNetlistText(c.getField("mouser#")),
        fromNetlistText( c.getValue() ),
        fromNetlistText( c.getPartName() ),
        fromNetlistText( c.getFootprint() ),
        mouser_price, mouser_availability
        ])
out.writerow(["Gesamtpreis: ", total_cost])




