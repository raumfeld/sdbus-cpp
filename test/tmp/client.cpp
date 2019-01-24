#include <systemd/sd-bus.h>
#include <stdexcept>
#include <map>
#include <iostream>

int main()
{
    int r{};
    
    int clientID = 1;
    
    /* Open system bus */
    sd_bus *bus;
    r = sd_bus_open_system(&bus);
    if (r < 0)
        throw std::runtime_error(strerror(-r));
    
    sd_bus_message *reply{};
    sd_bus_error error = SD_BUS_ERROR_NULL;
    
    std::cout << "Issuing call with client ID " << clientID << "... " << std::endl;

	/* Issue call */
    r = sd_bus_call_method(bus,
                            "com.kistler.testsdbuscpp",          /* service to contact */
                            "/com/kistler/dbusapp",         /* object path */
                            "com.kistler.dbusapp.problems", /* interface name */
                            "getProblem",                   /* method name */
                            &error,                         /* object to return error in */
                            &reply,                         /* return message on success */
                            "i",                            /* input signature */
                            clientID                        /* first argument */);
    if (r < 0)
        throw std::runtime_error(strerror(-r));
        
    r = sd_bus_message_enter_container(reply, SD_BUS_TYPE_ARRAY, "{ss}");
        
    char* key;
    char* value;
    std::map<std::string, std::string> problem;
    while ((r = sd_bus_message_read(reply, "{ss}", &key, &value)) > 0)
        problem.emplace(key, value);
    if (r < 0)
        throw std::runtime_error(strerror(-r));
        
    r = sd_bus_message_exit_container(reply);
    if (r < 0)
        throw std::runtime_error(strerror(-r));
    
    // REAL WORK ON PROBLEMS HERE >>>>
    std::cout << "Got reply: ";
    for (const auto& item : problem)
        std::cout << "{" << item.first << " : " << item.second << "} ";
    std::cout << std::endl << std::endl;
    // <<<<
    
    /* Cleanup */
    sd_bus_error_free(&error);
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);
}
