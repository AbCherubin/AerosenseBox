#include "Ab_LCD.h"

void SerialLCD::popup_reconnecting_on()

{
    close_win("Popup_reconnect");
    open_win("Popup_reconnect");
    set_visible("Popup_reconnect", "true");
}

void SerialLCD::popup_reconnecting_off()
{
    set_visible("Popup_reconnect", "false");
}

void SerialLCD::refreshData()
{
    // if the page is not Flight list then return
    if (page != 3)
    {
        return;
    }

    Serial.println(flight_list);
    set_visible("Overlay_flight", "false");
    if (flight_list == "")
    {
        set_text("label", "label_current_page", "1");
        set_text("label", "label_max_page", "/1");
        return;
    }

    open_win("Popup_loading");
    set_visible("Popup_loading", "true");
    buttonsPerPage = 3;
    numPages = (flight_list_size + buttonsPerPage - 1) / buttonsPerPage;
    currentJobPage = 1;

    char label_current_page_Str[3];
    snprintf(label_current_page_Str, sizeof(label_current_page_Str), "%d", numPages);
    set_max("label", "label_current_page", label_current_page_Str);

    char label_max_page_Str[4];
    snprintf(label_max_page_Str, sizeof(label_max_page_Str), "/%d", numPages);
    set_text("label", "label_max_page", label_max_page_Str);

    char currentJobPageStr[3];
    snprintf(currentJobPageStr, sizeof(currentJobPageStr), "%d", currentJobPage);
    set_text("label", "label_current_page", currentJobPageStr);

    set_visible("button1", "true");
    set_visible("button2", "true");
    set_visible("button3", "true");

    set_display_data(1);
    set_visible("Flight_list", "true");
    set_visible("Popup_loading", "false");
    set_visible("Overlay_flight", "true");
}

void SerialLCD::set_display_data(uint8_t page)
{

    DynamicJsonDocument _flight_list(ESP.getMaxAllocHeap() - 1024);
    DeserializationError error = deserializeJson(_flight_list, flight_list);
    _flight_list.shrinkToFit();
    if (error)
    {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }

    uint8_t block = 1;
    // p2
    uint8_t startIdx = (page - 1) * buttonsPerPage;
    uint8_t endIdx = std::min(static_cast<size_t>(page * buttonsPerPage), static_cast<size_t>(flight_list_size));

    for (int i = startIdx, j = 1; i < endIdx; i++, j++)
    {

        // char NoStr[3];
        // snprintf(NoStr, sizeof(NoStr), "%d", i + 1);
        // char label_button_no[30];
        // snprintf(label_button_no, sizeof(label_button_no), "label_button%d_%d_No", j, block);
        // set_text("label", label_button_no, NoStr);

        const char *flightNumber = _flight_list[i]["flight"];
        char label_button_flight[30];
        snprintf(label_button_flight, sizeof(label_button_flight), "label_button%d_%d_flight", j, block);
        set_text("label", label_button_flight, const_cast<char *>(flightNumber));

        const char *ST = _flight_list[i]["ST"];
        char label_button_ST[30];
        snprintf(label_button_ST, sizeof(label_button_ST), "label_button%d_%d_ST", j, block);
        set_text("label", label_button_ST, const_cast<char *>(ST));

        const char *ET = _flight_list[i]["ET"];
        char label_button_ET[30];
        snprintf(label_button_ET, sizeof(label_button_ET), "label_button%d_%d_ET", j, block);
        set_text("label", label_button_ET, const_cast<char *>(ET));

        if (flight_type == "DEP")
        {
            const char *pickup = _flight_list[i]["gate"];
            char label_button_pickup[30];
            snprintf(label_button_pickup, sizeof(label_button_pickup), "label_button%d_%d_pickup", j, block);
            set_text("label", label_button_pickup, const_cast<char *>(pickup));

            const char *dropoff = _flight_list[i]["bay"];
            char label_button_dropoff[30];
            snprintf(label_button_dropoff, sizeof(label_button_dropoff), "label_button%d_%d_dropoff", j, block);
            set_text("label", label_button_dropoff, const_cast<char *>(dropoff));
        }
        else
        {
            const char *pickup = _flight_list[i]["bay"];
            char label_button_pickup[30];
            snprintf(label_button_pickup, sizeof(label_button_pickup), "label_button%d_%d_pickup", j, block);
            set_text("label", label_button_pickup, const_cast<char *>(pickup));

            const char *dropoff = _flight_list[i]["gate"];
            char label_button_dropoff[30];
            snprintf(label_button_dropoff, sizeof(label_button_dropoff), "label_button%d_%d_dropoff", j, block);
            set_text("label", label_button_dropoff, const_cast<char *>(dropoff));
        }
    }
    if (page == numPages)
    {
        for (int k = endIdx + 1; k <= page * buttonsPerPage; k++)
        {
            char set_visible_row[25];
            snprintf(set_visible_row, sizeof(set_visible_row), "button%d", (((k - 1) % 3) + 1));
            set_visible(set_visible_row, "false");
        }
    }
    else if (page == numPages - 1)
    {
        for (int k = 1; k <= buttonsPerPage; k++)
        {
            char set_visible_row[25];
            snprintf(set_visible_row, sizeof(set_visible_row), "button%d", (k));
            set_visible(set_visible_row, "true");
        }
    }
}
void SerialLCD::Initial_setup()
{
    char GSEIdStr[10];
    snprintf(GSEIdStr, sizeof(GSEIdStr), "%s", GSEId.c_str());

    char DatetimeStr[20];
    snprintf(DatetimeStr, sizeof(DatetimeStr), "%s", Datetime.c_str());

    open_win("RFID");
    set_visible("RFID", "true");
    open_win("Popup_nodata");
    set_visible("Popup_nodata", "false");
    open_win("Flight_arr_dep");
    set_visible("Flight_arr_dep", "false");
    open_win("Flight_list");
    set_visible("Flight_list", "false");
    set_date("rfid_digit_date", DatetimeStr);
    set_visible("rfid_digit_date", "true");
    set_visible("rfid_digit_time", "true");
    set_visible("label_gse", "false");

    set_text("label", "label_gse", GSEIdStr);

    set_visible("label_gse", "true");
    set_sys("sys_version");
    page = 1;
    char localPageStr[3];
    snprintf(localPageStr, sizeof(localPageStr), "%d", page);
    set_text("label", "local_page", localPageStr);
}

void SerialLCD::page0()
{
    if (timer_flag == 1)
    {
        // set_sys("sys_hello");
        timer_flag = 0;
        get_text("label", "local_rfid");
        // get_text("label", "local_page");
        set_sys("sys_hello");
    }

    if (receive_over_flage == 1 && GSEId != "" && Datetime != "" && isInitalTaskReady)
    {

        if (STONER.widget != NULL && strcmp(STONER.widget, "local_rfid") == 0)
        {

            // Check selected_flight
            String widget;
            char local_rfid[20];
            int name_lum, status_lum;
            widget = (char *)STONER.widget;
            name_lum = strlen(STONER.widget);
            status_lum = STONER.len - name_lum - 3;
            if (status_lum < 0)
                return;

            memset(local_rfid, NULL, 20);
            memcpy(local_rfid, STONER.text, status_lum);

            String local_rfid_str(local_rfid);

            if (local_rfid_str != "")
            {
                Serial.print("local_rfid:");
                Serial.println(local_rfid_str);
                employeeId = local_rfid_str;
            }
            get_text("label", "local_type");
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "local_type") == 0)
        {

            String widget;
            char local_type[20];
            int name_lum, status_lum;
            widget = (char *)STONER.widget;
            name_lum = strlen(STONER.widget);
            status_lum = STONER.len - name_lum - 3;
            if (status_lum < 0)
                return;
            memset(local_type, NULL, 20);
            memcpy(local_type, STONER.text, status_lum);

            String local_type_str(local_type);

            if (local_type_str != "")
            {
                Serial.print("local_type:");
                Serial.println(local_type_str);
                flight_type = local_type_str;
            }
            get_text("label", "local_page");
        }
        // if not initial starting (restarted)
        else if (STONER.widget != NULL && strcmp(STONER.widget, "local_page") == 0)
        {
            String widget;
            char local_page[20];
            int name_lum, status_lum;
            widget = (char *)STONER.widget;
            name_lum = strlen(STONER.widget);
            status_lum = STONER.len - name_lum - 3;
            if (status_lum < 0)
                return;

            memset(local_page, NULL, 20);
            memcpy(local_page, STONER.text, status_lum);

            String local_page_str(local_page);

            int intValue = local_page_str.toInt();
            Serial.print("local_page:");
            Serial.println(local_page_str);
            if (intValue > 1 && intValue <= 255)
            {
                recheck_flight_list = true;
                uint8_t uint8Value = static_cast<uint8_t>(intValue);
                page = uint8Value;
                // if the page is selecting flight(4) then back to flight list(3) page
                if (page == 4)
                {
                    set_visible("Accept_flight", "false");

                    page = 3;
                    char localPageStr[3];
                    snprintf(localPageStr, sizeof(localPageStr), "%d", page);
                    set_text("label", "local_page", localPageStr);
                }
                // if (taskId && selected_flight)
                // {
                //     page = 4;
                //     isSelectFlight_Ok = true;
                // }
                // if the page is in task page(5) but no task then back to flight list
                if (page == 5 && taskId == "")
                {

                    isCancelTask_Ok = true;
                }
                Serial.print("Task ID: ");
                Serial.println(taskId);
                Serial.print("Page: ");
                Serial.println(page);
                set_sys("sys_version");
            }
            else
            {
                Initial_setup();
            }
            popup_reconnecting_off();
        }
        else
        {

            // Initial setup

            Initial_setup();
            popup_reconnecting_off();
        }

        receive_over_flage = 0;
    }
}

// RFID
void SerialLCD::page1()
{
    if (isLogin)
    {
        // open_win("Flight_list");
        // set_visible("Flight_list", "true");
        // open_win("overlay_flight_page1");

        set_visible("Flight_arr_dep", "true");

        char driverBuffer[64];
        snprintf(driverBuffer, sizeof(driverBuffer), "%s", Driver.c_str());
        set_text("label", "label_user_data", driverBuffer);

        char localrfidStr[20];
        snprintf(localrfidStr, sizeof(localrfidStr), "%s", employeeId.c_str());
        set_text("label", "local_rfid", localrfidStr);

        page = 2;
        char localPageStr[3];
        snprintf(localPageStr, sizeof(localPageStr), "%d", page);
        set_text("label", "local_page", localPageStr);

        // refreshData();
        set_visible("RFID", "false");
    }
    else if (driverLoginFailed || visibilityInProgress)
    {
        if (!visibilityInProgress)
        {
            set_visible("Popup_nodata", "true");

            visibilityStartTime = millis();
            visibilityInProgress = true;
        }

        // Check if the visibility duration has passed
        if (visibilityInProgress && millis() - visibilityStartTime >= visibilityDuration)
        {
            // After 2 seconds, set visibility to false
            set_visible("Popup_nodata", "false");
            visibilityInProgress = false;
            driverLoginFailed = false;
        }
    }

    if (receive_over_flage == 1)
    {
        // if (STONER.widget != NULL && strcmp(STONER.widget, "button_exit_overlay_rfid") == 0)
        // {
        //     set_visible("RFID", "false");

        //     page = 1;
        //     char localPageStr[3];
        //     snprintf(localPageStr, sizeof(localPageStr), "%d", page);
        //     set_text("label", "local_page", localPageStr);

        //     isLogin = false;
        // }
        if (STONER.widget != NULL && strcmp(STONER.widget, "btn_setting_rfid") == 0)
        {
            open_win("Setting_page");
            set_visible("Setting_page", "true");

            String HMI_version = "HMI Device Version: " + String(STONER.data);
            set_text("label", "label_HMI_ver", const_cast<char *>(HMI_version.c_str()));
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "radio_btn_80") == 0)
        {
            set_brightness("80");
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "radio_btn_100") == 0)
        {
            set_brightness("100");
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "btn_setting_exit") == 0)
        {
            set_visible("Setting_page", "false");
        }
        receive_over_flage = 0;
    }
}
// Select ARR / DEP
void SerialLCD::page2()
{

    if (receive_over_flage == 1)
    {

        if (STONER.widget != NULL && strcmp(STONER.widget, "btn_arr") == 0)
        {
            set_visible("Flight_list", "true");
            open_win("Overlay_flight");

            flight_type = "ARR";
            char localTypeStr[5];
            snprintf(localTypeStr, sizeof(localTypeStr), "%s", flight_type);
            set_text("label", "local_type", localTypeStr);

            page = 3;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);

            set_text("label", "label_head_ST", "STA");
            set_text("label", "label_head_ET", "ETA");
            set_text("label", "label_head_pickup", "BAY");
            set_text("label", "label_head_dropoff", "GATE");

            set_visible("label_head_ST", "true");
            set_visible("label_head_ET", "true");
            set_visible("label_head_pickup", "true");
            set_visible("label_head_dropoff", "true");

            recheck_flight_list = true;
            set_visible("Overlay_flight", "false");
            open_win("Popup_loading");
            set_visible("Popup_loading", "true");
            loadingStartTime = millis();
            loadingInProgress = true;

            set_visible("Flight_arr_dep", "false");
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "btn_dep") == 0)
        {

            set_visible("Flight_list", "true");
            open_win("Overlay_flight");

            flight_type = "DEP";
            char localTypeStr[5];
            snprintf(localTypeStr, sizeof(localTypeStr), "%s", flight_type);
            set_text("label", "local_type", localTypeStr);

            page = 3;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);

            set_text("label", "label_head_ST", "STD");
            set_text("label", "label_head_ET", "ETD");
            set_text("label", "label_head_pickup", "GATE");
            set_text("label", "label_head_dropoff", "BAY");

            set_visible("label_head_ST", "true");
            set_visible("label_head_ET", "true");
            set_visible("label_head_pickup", "true");
            set_visible("label_head_dropoff", "true");

            recheck_flight_list = true;
            set_visible("Overlay_flight", "false");
            open_win("Popup_loading");
            set_visible("Popup_loading", "true");
            loadingStartTime = millis();
            loadingInProgress = true;

            set_visible("Flight_arr_dep", "false");
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "btn_flight_arr_dep_exit") == 0)
        {
            set_visible("RFID", "true");
            set_visible("Flight_arr_dep", "false");

            page = 1;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);

            set_text("label", "local_rfid", "");

            isLogin = false;

            flight_type = "";
            char localTypeStr[5];
            snprintf(localTypeStr, sizeof(localTypeStr), "%s", flight_type);
            set_text("label", "local_type", localTypeStr);
        }

        receive_over_flage = 0;
    }
}
// Flight list
void SerialLCD::page3()
{

    // Check if the loading Flight duration has passed
    if (loadingInProgress && millis() - loadingStartTime >= loadingDuration)
    {

        set_visible("Popup_loading", "false");
        loadingInProgress = false;
    }

    if (receive_over_flage == 1)
    {
        if (STONER.widget != NULL && strcmp(STONER.widget, "btn_arrow_right") == 0)
        {
            if (currentJobPage < numPages)
            {
                currentJobPage++;
                set_display_data(currentJobPage);

                char currentJobPageStr[3];
                snprintf(currentJobPageStr, sizeof(currentJobPageStr), "%d", currentJobPage);
                set_text("label", "label_current_page", currentJobPageStr);
            }
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "btn_arrow_left") == 0)
        {
            if (currentJobPage > 1)
            {
                currentJobPage--;
                set_display_data(currentJobPage);
                char currentJobPageStr[3];
                snprintf(currentJobPageStr, sizeof(currentJobPageStr), "%d", currentJobPage);
                set_text("label", "label_current_page", currentJobPageStr);
            }
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "btn_refresh") == 0)
        {
            recheck_flight_list = true;
            set_visible("Overlay_flight", "false");
            set_visible("Popup_loading", "true");
            loadingStartTime = millis();
            loadingInProgress = true;
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "btn_logout") == 0)
        {
            set_visible("Overlay_flight", "false");
            set_visible("Flight_list", "false");

            flight_type = "";
            char localTypeStr[5];
            snprintf(localTypeStr, sizeof(localTypeStr), "%s", flight_type);
            set_text("label", "local_type", localTypeStr);

            page = 1;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);

            set_text("label", "local_rfid", "");

            isLogin = false;
            set_visible("RFID", "true");
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "btn_arrow_back") == 0)
        {
            set_visible("Overlay_flight", "false");
            set_visible("Flight_list", "false");
            set_visible("Flight_arr_dep", "true");

            flight_type = "";
            char localTypeStr[5];
            snprintf(localTypeStr, sizeof(localTypeStr), "%s", flight_type);
            set_text("label", "local_type", localTypeStr);

            page = 2;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);
        }
        else if (STONER.widget != NULL && isButtonPageWidget(STONER.widget))
        {
            Serial.println(selected_flight);
            StaticJsonDocument<256> _selected_flight;
            DeserializationError error = deserializeJson(_selected_flight, selected_flight);
            if (error)
            {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }
            isSelectFlight_Ok = false;
            open_win("Accept_flight");

            if (flight_type == "DEP")
            {
                const char *flightNumber = _selected_flight["flight"];
                String flightNumberString = "Accept Flight: " + String(flightNumber);
                set_text("label", "label_accept_flight_data", const_cast<char *>(flightNumberString.c_str()));

                const char *ST = _selected_flight["ST"];
                String STString = "STD: " + String(ST);
                set_text("label", "label_accept_flight_ST_data", const_cast<char *>(STString.c_str()));

                const char *ET = _selected_flight["ET"];
                String ETString = "ETD: " + String(ET);
                set_text("label", "label_accept_flight_ET_data", const_cast<char *>(ETString.c_str()));

                const char *gate = _selected_flight["gate"];
                String gateString = "Pick Up: " + String(gate);
                set_text("label", "label_accept_flight_pickup_data", const_cast<char *>(gateString.c_str()));

                const char *bay = _selected_flight["bay"];
                String bayString = "Drop Off: " + String(bay);
                set_text("label", "label_accept_flight_dropoff_data", const_cast<char *>(bayString.c_str()));
            }
            else
            {
                const char *flightNumber = _selected_flight["flight"];
                String flightNumberString = "Accept Flight: " + String(flightNumber);
                set_text("label", "label_accept_flight_data", const_cast<char *>(flightNumberString.c_str()));

                const char *ST = _selected_flight["ST"];
                String STString = "STA: " + String(ST);
                set_text("label", "label_accept_flight_ST_data", const_cast<char *>(STString.c_str()));

                const char *ET = _selected_flight["ET"];
                String ETString = "ETA: " + String(ET);
                set_text("label", "label_accept_flight_ET_data", const_cast<char *>(ETString.c_str()));

                const char *bay = _selected_flight["bay"];
                String bayString = "Pick Up: " + String(bay);
                set_text("label", "label_accept_flight_pickup_data", const_cast<char *>(bayString.c_str()));

                const char *gate = _selected_flight["gate"];
                String gateString = "Drop Off: " + String(gate);
                set_text("label", "label_accept_flight_dropoff_data", const_cast<char *>(gateString.c_str()));
            }

            set_visible("label_accept_flight_data", "true");
            set_visible("label_accept_flight_ST_data", "true");
            set_visible("label_accept_flight_ET_data", "true");
            set_visible("label_accept_flight_pickup_data", "true");
            set_visible("label_accept_flight_dropoff_data", "true");

            set_visible("Accept_flight", "true");
            page = 4;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);

            _selected_flight.clear();
        }

        receive_over_flage = 0;
    }
}

// Select Flight
void SerialLCD::page4()
{
    if (isSelectFlight_Ok)
    {

        StaticJsonDocument<256> _selected_flight;
        DeserializationError error = deserializeJson(_selected_flight, selected_flight);

        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }

        job_step = 0;
        currentRound = 1;

        open_win("Blank");
        set_visible("Blank", "true");

        open_win("Top_overlay_status");
        open_win("Bottom_overlay");

        open_win("Center_overlay_confirm");
        set_visible("Center_overlay_confirm", "true");

        open_win("Center_overlay_pickup");
        set_visible("Center_overlay_pickup", "false");

        open_win("Center_overlay_dropoff");
        set_visible("Center_overlay_dropoff", "false");

        open_win("Center_overlay_finished");
        set_visible("Center_overlay_finished", "false");

        set_visible("Top_overlay_status", "false");
        set_visible("Bottom_overlay", "false");

        open_win("Round_overlay");
        if (flight_type == "DEP")
        {
            const char *flightNumber = _selected_flight["flight"];
            String flightNumberString = "Flight: " + String(flightNumber);
            set_text("label", "label_ovl_flight_data", const_cast<char *>(flightNumberString.c_str()));

            const char *ST = _selected_flight["ST"];
            String STString = "STD: " + String(ST);
            set_text("label", "label_ovl_ST_data", const_cast<char *>(STString.c_str()));

            const char *ET = _selected_flight["ET"];
            String ETString = "ETD: " + String(ET);
            set_text("label", "label_ovl_ET_data", const_cast<char *>(ETString.c_str()));

            const char *gate = _selected_flight["gate"];
            String gateString = String(gate);
            set_text("label", "label_pickup", const_cast<char *>(gateString.c_str()));

            const char *bay = _selected_flight["bay"];
            String bayString = String(bay);
            set_text("label", "label_dropoff", const_cast<char *>(bayString.c_str()));
        }
        else
        {
            const char *flightNumber = _selected_flight["flight"];
            String flightNumberString = "Flight: " + String(flightNumber);
            set_text("label", "label_ovl_flight_data", const_cast<char *>(flightNumberString.c_str()));

            const char *ST = _selected_flight["ST"];
            String STString = "STA: " + String(ST);
            set_text("label", "label_ovl_ST_data", const_cast<char *>(STString.c_str()));

            const char *ET = _selected_flight["ET"];
            String ETString = "ETA: " + String(ET);
            set_text("label", "label_ovl_ET_data", const_cast<char *>(ETString.c_str()));

            const char *bay = _selected_flight["bay"];
            String bayString = String(bay);
            set_text("label", "label_pickup", const_cast<char *>(bayString.c_str()));

            const char *gate = _selected_flight["gate"];
            String gateString = String(gate);
            set_text("label", "label_dropoff", const_cast<char *>(gateString.c_str()));
        }

        set_visible("label_ovl_flight_data", "true");
        set_visible("label_ovl_ST_data", "true");
        set_visible("label_ovl_ET_data", "true");
        set_visible("label_pickup", "true");
        set_visible("label_dropoff", "true");
        // set_visible("label_gohome", "false");

        set_visible("button_dropoff_pickup", "true");
        set_enable("button_dropoff_pickup", "true");
        set_visible("button_dropoff_finished", "false");
        set_enable("button_dropoff_finished", "false");

        set_text("button", "button_dropoff_pickup", "Pick Up");

        // char driverBuffer[64];
        // snprintf(driverBuffer, sizeof(driverBuffer), "Driver: %s", Driver.c_str());
        // set_text("label", "label_gohome", driverBuffer);
        // set_visible("label_gohome", "true");

        // set_visible("label_gohome_data", "false");

        set_visible("Top_overlay_status", "true");
        set_visible("Bottom_overlay", "true");
        set_visible("Overlay_flight", "false");

        set_visible("Accept_flight", "false");

        step = 0;
        startTime = 0;
        page = 5;
        char localPageStr[3];
        snprintf(localPageStr, sizeof(localPageStr), "%d", page);
        set_text("label", "local_page", localPageStr);

        _selected_flight.clear();
        isStepAction = true;
        isSelectFlight_Ok = false;
    }

    if (receive_over_flage == 1)
    {
        if (STONER.widget != NULL && strcmp(STONER.widget, "button_accept_no") == 0)
        {

            set_visible("Accept_flight", "false");

            page = 3;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);

            if (recheck_flight_list)
            {
                refreshData();
                recheck_flight_list = false;
            }
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "button_accept_yes") == 0)
        {
            StaticJsonDocument<256> _selected_flight;
            DeserializationError error = deserializeJson(_selected_flight, selected_flight);

            if (error)
            {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }

            int id = _selected_flight["id"];
            taskId = String(id);
            isSelectFlight = true;
            Serial.println(taskId);
        }

        receive_over_flage = 0;
    }
}
// Tasks
void SerialLCD::page5()
{

    // open_win("Round_overlay");
    if (isCancelTask_Ok)
    {
        page = 3;
        char localPageStr[3];
        snprintf(localPageStr, sizeof(localPageStr), "%d", page);
        set_text("label", "local_page", localPageStr);
        close_win("Round_overlay");
        set_visible("Top_overlay_status", "false");
        set_visible("Bottom_overlay", "false");
        set_visible("Center_overlay_confirm", "false");
        set_visible("Center_overlay_pickup", "false");
        set_visible("Center_overlay_dropoff", "false");
        set_visible("Center_overlay_finished", "false");
        set_visible("Cancel_flight", "false");
        set_visible("Blank", "false");
        refreshData();

        isCancelTask_Ok = false;
        return;
    }

    if (isStepAction_Ok)
    {

        if (step == 0) // Initial step 0
        {
            set_color("label_pickup", "bg_color", "4290098613");
            set_color("label_dropoff", "bg_color", "4290098613");
            step++;
        }
        else if (step == 1 || step == 3 || step == 5) // step 1,3,5
        {
            set_visible("button_dropoff_finished", "false");
            set_enable("button_dropoff_finished", "false");

            set_text("button", "button_dropoff_pickup", "Drop Off");
            job_step = 1;

            set_color("label_pickup", "bg_color", "4282824813");
            set_color("label_dropoff", "bg_color", "4290098613");

            step++;
        }
        else if (step == 2 || step == 4) // step 2,4
        {
            set_visible("button_dropoff_finished", "true");
            set_enable("button_dropoff_finished", "true");

            set_text("button", "button_dropoff_pickup", "Next Round");
            job_step = 2;

            set_color("label_pickup", "bg_color", "4282824813");
            set_color("label_dropoff", "bg_color", "4282824813");

            step++;
        }
        else if (step == 6) // last Step No next Round button step 6
        {
            set_visible("button_dropoff_pickup", "false");
            set_enable("button_dropoff_pickup", "false");
            set_visible("button_dropoff_finished", "true");
            set_enable("button_dropoff_finished", "true");
            job_step = 2;

            set_color("label_pickup", "bg_color", "4282824813");
            set_color("label_dropoff", "bg_color", "4282824813");

            step++;
        }
        else if (step == 7) // Finish Job Go to Flight list page step 7
        {
            job_step = 3;
        }

        isStepAction_Ok = false;
    }

    if (job_step == 0)
    {

        set_visible("Center_overlay_confirm", "true");
    }
    else if (job_step == 1)
    {

        set_visible("Center_overlay_pickup", "true");
    }
    else if (job_step == 2)
    {

        set_visible("Center_overlay_dropoff", "true");
    }
    else if (job_step == 3)
    {
        set_visible("Center_overlay_finished", "true");
        // set_visible("label_gohome", "true");
        // set_visible("label_gohome_data", "true");
        // set_text("label", "label_gohome", "... Return in");
        unsigned long now = millis();
        // if (startTime == 0)
        // {
        //     startTime = now;
        // }
        unsigned long elapsedTime = now - startTime;

        if (elapsedTime >= countdownDuration)
        {
            page = 2;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);

            // set_text("label", "label_gohome_data", "0");
            close_win("Round_overlay");

            // set_visible("label_gohome", "false");
            // set_visible("label_gohome_data", "false");

            set_visible("Center_overlay_confirm", "false");
            set_visible("Center_overlay_pickup", "false");
            set_visible("Center_overlay_dropoff", "false");

            set_visible("Top_overlay_status", "false");
            set_visible("Bottom_overlay", "false");
            set_visible("Center_overlay_finished", "false");
            set_visible("Cancel_flight", "false");
            set_visible("Blank", "false");

            // recheck_flight_list = true;
            // set_visible("Overlay_flight", "false");
            // set_visible("Popup_loading", "true");
            // loadingStartTime = millis();
            // loadingInProgress = true;
            set_visible("Overlay_flight", "false");
            set_visible("Flight_list", "false");
            set_visible("Flight_arr_dep", "true");

            flight_type = "";
            char localTypeStr[5];
            snprintf(localTypeStr, sizeof(localTypeStr), "%s", flight_type);
            set_text("label", "local_type", localTypeStr);
        }
        else
        {
            unsigned long remainingTime = (countdownDuration - elapsedTime) / 1000;
            char remainingTimeStr[3];
            // snprintf(remainingTimeStr, sizeof(remainingTimeStr), "%lu", remainingTime);
            // set_text("label", "label_gohome_data", remainingTimeStr);
        }
    }

    if (receive_over_flage == 1)
    {
        if (STONER.widget != NULL && (strcmp(STONER.widget, "btn_ovl_exit") == 0))
        {
            open_win("Cancel_flight");
            char flightNumber[20];
            snprintf(flightNumber, sizeof(flightNumber), "%s", currentFlight.c_str());
            set_text("label", "label_cancel_flight_data", flightNumber);
            set_visible("Round_overlay", "false");
            set_visible("Cancel_flight", "true");
        }
        else if (STONER.widget != NULL && (strcmp(STONER.widget, "button_cancel_yes") == 0))
        {
            isCancelTask = true;
        }
        else if (STONER.widget != NULL && (strcmp(STONER.widget, "button_cancel_no") == 0))
        {
            set_visible("Cancel_flight", "false");
            set_visible("Round_overlay", "true");
        }
        else if (STONER.widget != NULL && (strcmp(STONER.widget, "button_dropoff_pickup") == 0))
        {
            if (job_step == 2) // Next Round
            {
                currentRound++;
                set_visible("Center_overlay_pickup", "false");
                set_visible("Center_overlay_dropoff", "false");

                set_text("button", "button_dropoff_pickup", "Pick Up");
                job_step = 0;
                // close_win("round_overlay");
                char currentRoundStr[15];
                snprintf(currentRoundStr, sizeof(currentRoundStr), "Round: %d", currentRound);
                set_text("label", "round_label", currentRoundStr);

                set_color("label_pickup", "bg_color", "4290098613");
                set_color("label_dropoff", "bg_color", "4290098613");
            }
            else
            {
                isStepAction = true;
            }
        }
        else if (STONER.widget != NULL && (strcmp(STONER.widget, "button_dropoff_finished") == 0))
        {
            isStepAction = true;
            step = 7;
        }
        receive_over_flage = 0;
    }
}

bool SerialLCD::isButtonPageWidget(const char *widgetName)
{
    Serial.println(widgetName);
    int dummyInt1;
    if (sscanf(widgetName, "button%d", &dummyInt1) == 1)
    {
        Serial.println(dummyInt1);
        selected_flight = "";
        DynamicJsonDocument _flight_list(ESP.getMaxAllocHeap() - 1024);
        DeserializationError error = deserializeJson(_flight_list, flight_list);
        _flight_list.shrinkToFit();
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return false;
        }
        int index = ((currentJobPage - 1) * 3) + (dummyInt1 - 1);

        if (index >= 0 && index < _flight_list.size())
        {
            serializeJson(_flight_list[index], selected_flight);
        }
        else
        {
            Serial.println("Index out of bounds!");

            return false;
        }
        _flight_list.clear();
        return true;
    }
    else
    {
        return false;
    }
}
