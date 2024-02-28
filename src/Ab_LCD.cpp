#include "Ab_LCD.h"

void SerialLCD::popup_reconnecting_on()

{
    close_win("popup_reconnect");
    open_win("popup_reconnect");
    set_visible("popup_reconnect", "true");
}

void SerialLCD::popup_reconnecting_off()
{
    set_visible("popup_reconnect", "false");
}

void SerialLCD::refreshData()
{
    if (page != 3)
    {
        return;
    }
    Serial.println(flight_list);
    set_visible("overlay_flight_page1", "false");
    if (flight_list == "")
    {
        set_text("label", "label_current_page", "1");
        set_text("label", "label_max_page", "/1");
        return;
    }

    open_win("popup_loading_1");
    set_visible("popup_loading_1", "true");
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

    set_visible("button1_page1", "true");
    set_visible("button2_page1", "true");
    set_visible("button3_page1", "true");

    set_display_data(1);
    set_visible("Flight_list", "true");
    set_visible("popup_loading_1", "false");
    set_visible("overlay_flight_page1", "true");
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

        char NoStr[3];
        snprintf(NoStr, sizeof(NoStr), "%d", i + 1);
        char label_button_no[30];
        snprintf(label_button_no, sizeof(label_button_no), "label_button%d_%d_No", j, block);
        set_text("label", label_button_no, NoStr);

        const char *flightNumber = _flight_list[i]["flight"];
        char label_button_flight[30];
        snprintf(label_button_flight, sizeof(label_button_flight), "label_button%d_%d_Flight", j, block);
        set_text("label", label_button_flight, const_cast<char *>(flightNumber));

        const char *bay = _flight_list[i]["bay"];
        char label_button_bay[30];
        snprintf(label_button_bay, sizeof(label_button_bay), "label_button%d_%d_Bay", j, block);
        set_text("label", label_button_bay, const_cast<char *>(bay));

        const char *std = _flight_list[i]["std"];
        char label_button_std[30];
        snprintf(label_button_std, sizeof(label_button_std), "label_button%d_%d_Std", j, block);
        set_text("label", label_button_std, const_cast<char *>(std));

        const char *etd = _flight_list[i]["etd"];
        char label_button_etd[30];
        snprintf(label_button_etd, sizeof(label_button_etd), "label_button%d_%d_Etd", j, block);
        set_text("label", label_button_etd, const_cast<char *>(etd));

        const char *gate = _flight_list[i]["gate"];
        char label_button_gate[30];
        snprintf(label_button_gate, sizeof(label_button_gate), "label_button%d_%d_Gate", j, block);
        set_text("label", label_button_gate, const_cast<char *>(gate));
    }
    if (page == numPages) //  if (page == numPages || page == (numPages - 1))
    {
        for (int k = endIdx + 1; k <= page * buttonsPerPage; k++)
        {
            char set_visible_row[25];
            // snprintf(set_visible_row, sizeof(set_visible_row), "button%d_page%d", (((k - 1) % 3) + 1), ((numPages % 3) + 1));
            snprintf(set_visible_row, sizeof(set_visible_row), "button%d_page1", (((k - 1) % 3) + 1));
            set_visible(set_visible_row, "false");
        }
    }
    else if (page == numPages - 1) //  if (page == numPages || page == (numPages - 1))
    {
        for (int k = 1; k <= buttonsPerPage; k++)
        {
            char set_visible_row[25];
            snprintf(set_visible_row, sizeof(set_visible_row), "button%d_page1", (k));
            set_visible(set_visible_row, "true");
        }
    }
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
            get_text("label", "local_page");
        }
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

            if (intValue > 0 && intValue <= 255)
            {
                recheck_flight_list = true;
                uint8_t uint8Value = static_cast<uint8_t>(intValue);
                page = uint8Value;
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

                // if (page == 5 && taskId == "")
                // {

                //     isCancelTask_Ok = true;
                // }
                Serial.print("Task ID: ");
                Serial.println(taskId);
                Serial.print("Page: ");
                Serial.println(page);
            }
            else
            {
                page = 1;
                char localPageStr[3];
                snprintf(localPageStr, sizeof(localPageStr), "%d", page);
                set_text("label", "local_page", localPageStr);
            }
            popup_reconnecting_off();
        }
        else
        {

            // Initial setup
            char GSEIdStr[10];
            snprintf(GSEIdStr, sizeof(GSEIdStr), "%s", GSEId.c_str());
            char DatetimeStr[20];
            snprintf(DatetimeStr, sizeof(DatetimeStr), "%s", Datetime.c_str());

            open_win("Box_detail");
            set_visible("Box_detail", "false");
            set_visible("labelGSE_data", "false");
            set_visible("digit_date", "false");
            set_visible("digit_time", "false");

            set_text("label", "labelGSE_data", GSEIdStr);

            set_date("digit_date", DatetimeStr);

            set_visible("labelGSE_data", "true");
            set_visible("digit_date", "true");
            set_visible("digit_time", "true");
            set_visible("Box_detail", "true");

            page = 1;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);
            popup_reconnecting_off();
        }

        receive_over_flage = 0;
    }
}

void SerialLCD::page1()
{

    if (receive_over_flage == 1)
    {

        if (STONER.widget != NULL && strcmp(STONER.widget, "buttonJob") == 0)
        {
            open_win("RFID");
            set_visible("RFID", "true");
            open_win("popup_nodata");
            set_visible("popup_nodata", "false");
            page = 2;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);
        }

        receive_over_flage = 0;
    }
}

// RFID
void SerialLCD::page2()
{

    if (isLogin)
    {
        open_win("Flight_list");
        set_visible("Flight_list", "true");
        open_win("overlay_flight_page1");

        char driverBuffer[64];
        snprintf(driverBuffer, sizeof(driverBuffer), "%s", Driver.c_str());
        set_text("label", "label_user_data", driverBuffer);

        char localrfidStr[20];
        snprintf(localrfidStr, sizeof(localrfidStr), "%s", employeeId.c_str());
        set_text("label", "local_rfid", localrfidStr);

        page = 3;
        char localPageStr[3];
        snprintf(localPageStr, sizeof(localPageStr), "%d", page);
        set_text("label", "local_page", localPageStr);

        refreshData();
        set_visible("RFID", "false");
    }
    else if (driverLoginFailed || visibilityInProgress)
    {
        if (!visibilityInProgress)
        {
            set_visible("popup_nodata", "true");

            visibilityStartTime = millis();
            visibilityInProgress = true;
        }

        // Check if the visibility duration has passed
        if (visibilityInProgress && millis() - visibilityStartTime >= visibilityDuration)
        {
            // After 2 seconds, set visibility to false
            set_visible("popup_nodata", "false");
            visibilityInProgress = false;
            driverLoginFailed = false;
        }
    }
    if (receive_over_flage == 1)
    {
        if (STONER.widget != NULL && strcmp(STONER.widget, "button_exit_overlay_rfid") == 0)
        {
            set_visible("RFID", "false");

            page = 1;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);

            isLogin = false;
        }
        receive_over_flage = 0;
    }
}

void SerialLCD::page3()
{

    // Check if the loading Flight duration has passed
    if (loadingInProgress && millis() - loadingStartTime >= loadingDuration)
    {

        set_visible("popup_loading_1", "false");
        loadingInProgress = false;
    }

    if (receive_over_flage == 1)
    {
        if (STONER.widget != NULL && strcmp(STONER.widget, "button_arrow_right") == 0)
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
        else if (STONER.widget != NULL && strcmp(STONER.widget, "button_arrow_left") == 0)
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
        else if (STONER.widget != NULL && strcmp(STONER.widget, "button_refresh") == 0)
        {
            recheck_flight_list = true;
            set_visible("overlay_flight_page1", "false");
            set_visible("popup_loading_1", "true");
            loadingStartTime = millis();
            loadingInProgress = true;
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "button_goback") == 0)
        {
            set_visible("overlay_flight_page1", "false");
            set_visible("Flight_list", "false");

            page = 1;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);

            set_text("label", "local_rfid", "");

            isLogin = false;
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

            const char *flightNumber = _selected_flight["flight"];
            set_text("label", "label_accept_flight_data", const_cast<char *>(flightNumber));

            const char *bay = _selected_flight["bay"];
            set_text("label", "label_accept_flight_bay_data", const_cast<char *>(bay));

            const char *std = _selected_flight["std"];
            set_text("label", "label_accept_flight_std_data", const_cast<char *>(std));

            const char *etd = _selected_flight["etd"];
            set_text("label", "label_accept_flight_etd_data", const_cast<char *>(etd));

            const char *gate = _selected_flight["gate"];
            set_text("label", "label_accept_flight_gate_data", const_cast<char *>(gate));

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

        open_win("blank");
        set_visible("blank", "true");

        open_win("top_overlay_status");
        open_win("bottom_overlay");

        open_win("center_overlay_confirm");
        set_visible("center_overlay_confirm", "true");

        open_win("center_overlay_pickup");
        set_visible("center_overlay_pickup", "false");

        open_win("center_overlay_dropoff");
        set_visible("center_overlay_dropoff", "false");

        open_win("center_overlay_finished");
        set_visible("center_overlay_finished", "false");

        set_visible("top_overlay_status", "false");
        set_visible("bottom_overlay", "false");

        const char *flightNumber = _selected_flight["flight"];
        set_text("label", "label_overlay_flight_data", const_cast<char *>(flightNumber));

        const char *bay = _selected_flight["bay"];
        set_text("label", "label_overlay_bay_data", const_cast<char *>(bay));

        const char *std = _selected_flight["std"];
        set_text("label", "label_overlay_std_data", const_cast<char *>(std));

        const char *etd = _selected_flight["etd"];
        set_text("label", "label_overlay_etd_data", const_cast<char *>(etd));

        const char *gate = _selected_flight["gate"];
        set_text("label", "label_overlay_gate_data", const_cast<char *>(gate));

        set_visible("label_gohome", "false");

        set_visible("button_dropoff_pickup", "true");
        set_enable("button_dropoff_pickup", "true");
        set_visible("button_dropoff_finished", "false");
        set_enable("button_dropoff_finished", "false");

        set_text("button", "button_dropoff_pickup", "Pick Up");

        char driverBuffer[64];
        snprintf(driverBuffer, sizeof(driverBuffer), "Driver: %s", Driver.c_str());
        set_text("label", "label_gohome", driverBuffer);
        set_visible("label_gohome", "true");

        set_visible("label_gohome_data", "false");

        set_visible("top_overlay_status", "true");
        set_visible("bottom_overlay", "true");

        set_visible("Accept_flight", "false");

        set_visible("overlay_flight_page1", "false");
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
void SerialLCD::page5()
{

    open_win("round_overlay");
    if (isCancelTask_Ok)
    {
        page = 3;
        char localPageStr[3];
        snprintf(localPageStr, sizeof(localPageStr), "%d", page);
        set_text("label", "local_page", localPageStr);
        close_win("round_overlay");
        set_visible("top_overlay_status", "false");
        set_visible("bottom_overlay", "false");
        set_visible("center_overlay_confirm", "false");
        set_visible("center_overlay_pickup", "false");
        set_visible("center_overlay_dropoff", "false");
        set_visible("center_overlay_finished", "false");
        set_visible("cancel_flight", "false");
        set_visible("blank", "false");
        refreshData();

        isCancelTask_Ok = false;
        return;
    }

    if (isStepAction_Ok)
    {

        if (step == 0) // Initial step 0
        {
            step++;
        }
        else if (step == 1 || step == 3 || step == 5) // step 1,3,5
        {
            set_visible("button_dropoff_finished", "false");
            set_enable("button_dropoff_finished", "false");

            set_text("button", "button_dropoff_pickup", "Drop Off");
            job_step = 1;
            step++;
        }
        else if (step == 2 || step == 4) // step 2,4
        {
            set_visible("button_dropoff_finished", "true");
            set_enable("button_dropoff_finished", "true");

            set_text("button", "button_dropoff_pickup", "Next Round");
            job_step = 2;
            step++;
        }
        else if (step == 6) // last Step No next Round button step 6
        {
            set_visible("button_dropoff_pickup", "false");
            set_enable("button_dropoff_pickup", "false");
            set_visible("button_dropoff_finished", "true");
            set_enable("button_dropoff_finished", "true");
            job_step = 2;
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

        set_visible("center_overlay_confirm", "true");
    }
    else if (job_step == 1)
    {

        set_visible("center_overlay_pickup", "true");
    }
    else if (job_step == 2)
    {

        set_visible("center_overlay_dropoff", "true");
    }
    else if (job_step == 3)
    {
        set_visible("center_overlay_finished", "true");
        set_visible("label_gohome", "true");
        set_visible("label_gohome_data", "true");
        set_text("label", "label_gohome", "... Return in");
        unsigned long now = millis();
        // if (startTime == 0)
        // {
        //     startTime = now;
        // }
        unsigned long elapsedTime = now - startTime;

        if (elapsedTime >= countdownDuration)
        {
            page = 3;
            char localPageStr[3];
            snprintf(localPageStr, sizeof(localPageStr), "%d", page);
            set_text("label", "local_page", localPageStr);

            set_text("label", "label_gohome_data", "0");
            close_win("round_overlay");

            set_visible("label_gohome", "false");
            set_visible("label_gohome_data", "false");

            set_visible("center_overlay_confirm", "false");
            set_visible("center_overlay_pickup", "false");
            set_visible("center_overlay_dropoff", "false");

            set_visible("top_overlay_status", "false");
            set_visible("bottom_overlay", "false");
            set_visible("center_overlay_finished", "false");
            set_visible("cancel_flight", "false");
            set_visible("blank", "false");
            refreshData();
        }
        else
        {
            unsigned long remainingTime = (countdownDuration - elapsedTime) / 1000;
            char remainingTimeStr[3];
            snprintf(remainingTimeStr, sizeof(remainingTimeStr), "%lu", remainingTime);
            set_text("label", "label_gohome_data", remainingTimeStr);
        }
    }

    if (receive_over_flage == 1)
    {
        if (STONER.widget != NULL && (strcmp(STONER.widget, "button_exit_overlay") == 0))
        {
            open_win("cancel_flight");
            char flightNumber[20];
            snprintf(flightNumber, sizeof(flightNumber), "%s", currentFlight.c_str());
            set_text("label", "label_cancel_flight_data", flightNumber);
            set_visible("round_overlay", "false");
            set_visible("cancel_flight", "true");
        }
        else if (STONER.widget != NULL && (strcmp(STONER.widget, "button_cancel_yes") == 0))
        {
            isCancelTask = true;
        }
        else if (STONER.widget != NULL && (strcmp(STONER.widget, "button_cancel_no") == 0))
        {
            set_visible("cancel_flight", "false");
            set_visible("round_overlay", "true");
        }
        else if (STONER.widget != NULL && (strcmp(STONER.widget, "button_dropoff_pickup") == 0))
        {
            if (job_step == 2) // Next Round
            {
                currentRound++;
                set_visible("center_overlay_pickup", "false");
                set_visible("center_overlay_dropoff", "false");

                set_text("button", "button_dropoff_pickup", "Pick Up");
                job_step = 0;
                // close_win("round_overlay");
                char currentRoundStr[15];
                snprintf(currentRoundStr, sizeof(currentRoundStr), "Round: %d", currentRound);
                set_text("label", "round_label", currentRoundStr);
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
    int dummyInt1, dummyInt2;
    if (sscanf(widgetName, "button%d_page%d", &dummyInt1, &dummyInt2) == 2)
    {
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