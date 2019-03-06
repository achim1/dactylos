
try:
    import visa
except ImportError:
    print ("Can not use NI_GPIB_USB controller, need to install visa library. Search pypi or github")

class NI_GPIB_USB(object):

    def __init__(self,gpib_adress=6, port=9999, publish=False):
        resource_manager = visa.ResourceManager()
        print("Found the following visa resources : {}".format(resource_manager.list_resources()))
        chamber_found = False
        try:
            self.resource = resource_manager.open_resource(resource_manager.list_resources()[0])
            resource_found = True
        except IndexError:
            print("Can not find any visa resources!")
            self.resource = None





