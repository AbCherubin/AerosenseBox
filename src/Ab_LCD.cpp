#include "Ab_LCD.h"

void SerialLCD::popup_loading_on()
{

    open_win("popup_loading_2");
    set_visible("popup_loading_2", "true");
}

void SerialLCD::popup_loading_off()
{
    set_visible("popup_loading_2", "false");
}

void SerialLCD::refreshData()
{
    if (page != 2)
    {
        return;
    }
    Serial.println(flight_list);
    set_visible("overlay_flight_page1", "false");
    set_visible("overlay_flight_page2", "false");
    set_visible("overlay_flight_page3", "false");
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

    char label_max_page_Str[3];
    snprintf(label_max_page_Str, sizeof(label_max_page_Str), "/%d", numPages);
    set_text("label", "label_max_page", label_max_page_Str);

    char currentJobPageStr[3];
    snprintf(currentJobPageStr, sizeof(currentJobPageStr), "%d", currentJobPage);
    set_text("label", "label_current_page", currentJobPageStr);

    set_visible("button1_page2", "true");
    set_visible("button2_page2", "true");
    set_visible("button3_page2", "true");

    set_visible("button1_page3", "true");
    set_visible("button2_page3", "true");
    set_visible("button3_page3", "true");

    set_visible("button1_page1", "true");
    set_visible("button2_page1", "true");
    set_visible("button3_page1", "true");

    set_display_data(1);
    set_display_data(2);
    set_visible("Flight_list", "true");
    set_visible("popup_loading_1", "false");
    set_visible("overlay_flight_page2", "true");
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

    uint8_t block = (page % 3) + 1;
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
    if (page == numPages || page == (numPages - 1))
    {
        for (int k = endIdx + 1; k <= page * buttonsPerPage; k++)
        {
            char set_visible_row[25];
            snprintf(set_visible_row, sizeof(set_visible_row), "button%d_page%d", (((k - 1) % 3) + 1), ((numPages % 3) + 1));
            set_visible(set_visible_row, "false");
        }
    }
    else if (numPages % 3 == page % 3)
    {
        for (int k = 1; k <= buttonsPerPage; k++)
        {
            char set_visible_row[25];
            snprintf(set_visible_row, sizeof(set_visible_row), "button%d_page%d", (k), ((numPages % 3) + 1));
            set_visible(set_visible_row, "true");
        }
    }
    _flight_list.clear();
}

void SerialLCD::page0()
{
    if (timer_flag == 1)
    {
        set_sys("sys_hello");
        timer_flag = 0;
        Serial.println("sys_hello");
    }

    if (receive_over_flage == 1 && GSEId != "")
    {
        char GSEIdStr[10];
        snprintf(GSEIdStr, sizeof(GSEIdStr), "%s", GSEId.c_str());
        char DatetimeStr[20];
        snprintf(DatetimeStr, sizeof(DatetimeStr), "%s", Datetime.c_str());

        set_visible("popup_nodata", "false");
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

        receive_over_flage = 0;

        page = 1;
    }
}

void SerialLCD::page1()
{

    if (receive_over_flage == 1)
    {

        if (STONER.widget != NULL && strcmp(STONER.widget, "buttonJob") == 0)
        {

            set_visible("popup_nodata", "false");
            open_win("Flight_list");
            set_visible("Flight_list", "true");
            open_win("overlay_flight_page1");
            open_win("overlay_flight_page2");
            open_win("overlay_flight_page3");

            page = 2;
            refreshData();
        }

        receive_over_flage = 0;
    }
}

void SerialLCD::page2()
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
                char set_visible_block[25];
                snprintf(set_visible_block, sizeof(set_visible_block), "overlay_flight_page%d", ((currentJobPage % 3) + 1));
                set_visible(set_visible_block, "false");

                currentJobPage++;

                char set_visible_block_next[25];
                snprintf(set_visible_block_next, sizeof(set_visible_block_next), "overlay_flight_page%d", ((currentJobPage % 3) + 1));
                set_visible(set_visible_block_next, "true");

                char currentJobPageStr[3];
                snprintf(currentJobPageStr, sizeof(currentJobPageStr), "%d", currentJobPage);
                set_text("label", "label_current_page", currentJobPageStr);

                if (currentJobPage < numPages)
                {
                    set_display_data(currentJobPage + 1);
                }
            }
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "button_arrow_left") == 0)
        {
            if (currentJobPage > 1)
            {
                char set_visible_block[25];
                snprintf(set_visible_block, sizeof(set_visible_block), "overlay_flight_page%d", ((currentJobPage % 3) + 1));
                set_visible(set_visible_block, "false");

                currentJobPage--;

                char currentJobPageStr[3];
                snprintf(currentJobPageStr, sizeof(currentJobPageStr), "%d", currentJobPage);
                set_text("label", "label_current_page", currentJobPageStr);

                char set_visible_block_next[25];
                snprintf(set_visible_block_next, sizeof(set_visible_block_next), "overlay_flight_page%d", ((currentJobPage % 3) + 1));
                set_visible(set_visible_block_next, "true");
                if (currentJobPage > 1)
                {
                    set_display_data(currentJobPage - 1);
                }
            }
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "button_refresh") == 0)
        {
            recheck_flight_list = true;
            set_visible("overlay_flight_page1", "false");
            set_visible("overlay_flight_page2", "false");
            set_visible("overlay_flight_page3", "false");
            set_visible("popup_loading_1", "true");
            loadingStartTime = millis();
            loadingInProgress = true;
        }
        else if (STONER.widget != NULL && strcmp(STONER.widget, "button_goback") == 0)
        {
            char set_visible_block1[25];
            snprintf(set_visible_block1, sizeof(set_visible_block1), "overlay_flight_page%d", ((currentJobPage % 3) + 1));
            set_visible(set_visible_block1, "false");
            set_visible("Flight_list", "false");
            page = 1;
        }
        else if (STONER.widget != NULL && isButtonPageWidget(STONER.widget))
        {

            DynamicJsonDocument _selected_flight(ESP.getMaxAllocHeap() - 1024);
            DeserializationError error = deserializeJson(_selected_flight, selected_flight);
            _selected_flight.shrinkToFit();
            if (error)
            {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }
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

            page = 3;
            _selected_flight.clear();
        }

        receive_over_flage = 0;
    }
}

void SerialLCD::page3()
{
    if (driverLoginFailed || visibilityInProgress)
    {
        if (!visibilityInProgress)
        {
            open_win("popup_nodata");
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
        if (STONER.widget != NULL && strcmp(STONER.widget, "button_accecpt_no") == 0)
        {

            set_visible("Accept_flight", "false");
            page = 2;
            if (recheck_flight_list)
            {
                refreshData();
                recheck_flight_list = false;
            }
        }

        receive_over_flage = 0;
    }
    if (isLogin)
    {

        DynamicJsonDocument _selected_flight(ESP.getMaxAllocHeap() - 1024);
        DeserializationError error = deserializeJson(_selected_flight, selected_flight);
        _selected_flight.shrinkToFit();
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }
        job_step = 0;

        open_win("blank");
        set_visible("blank", "true");

        open_win("top_overlay_status");
        open_win("bottom_overlay");

        open_win("center_overlay_confirm");
        set_visible("center_overlay_confirm", "true");

        open_win("center_overlay_standby");
        set_visible("center_overlay_standby", "false");

        open_win("center_overlay_push");
        set_visible("center_overlay_push", "false");

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

        char driverBuffer[64];
        snprintf(driverBuffer, sizeof(driverBuffer), "Driver: %s", Driver.c_str());
        set_text("label", "label_gohome", driverBuffer);
        set_visible("label_gohome", "true");

        set_visible("label_gohome_data", "false");

        set_visible("top_overlay_status", "true");
        set_visible("bottom_overlay", "true");

        set_visible("Accept_flight", "false");

        set_visible("overlay_flight_page1", "false");
        set_visible("overlay_flight_page2", "false");
        set_visible("overlay_flight_page3", "false");
        page = 4;
        _selected_flight.clear();
    }
}
void SerialLCD::page4()
{

    ////
    if (job_step == 0)
    {

        set_visible("center_overlay_confirm", "true");
    }
    else if (job_step == 1)
    {

        set_visible("center_overlay_standby", "true");
    }
    else if (job_step == 2)
    {
        set_visible("center_overlay_push", "true");
        startTime = millis();
    }
    else if (job_step == 3)
    {
        set_visible("center_overlay_finished", "true");
        set_visible("label_gohome", "true");
        set_visible("label_gohome_data", "true");
        set_text("label", "label_gohome", "... Return in");
        unsigned long now = millis();
        unsigned long elapsedTime = now - startTime;

        if (elapsedTime >= countdownDuration)
        {
            page = 2;

            set_text("label", "label_gohome_data", "0");
            set_visible("label_gohome", "false");
            set_visible("label_gohome_data", "false");
            set_visible("center_overlay_confirm", "false");
            set_visible("center_overlay_standby", "false");
            set_visible("center_overlay_push", "false");

            set_visible("top_overlay_status", "false");
            set_visible("bottom_overlay", "false");
            set_visible("center_overlay_finished", "false");
            set_visible("cancel_flight", "false");
            set_visible("blank", "false");
            refreshData();

            isLogin = false;
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

            // set_visible("cancel_flight", "false");

            DynamicJsonDocument _selected_flight(ESP.getMaxAllocHeap() - 1024);
            DeserializationError error = deserializeJson(_selected_flight, selected_flight);
            _selected_flight.shrinkToFit();
            if (error)
            {
                Serial.print(F("deserializeJson() failed: "));
                Serial.println(error.f_str());
                return;
            }
            job_step = 0;

            const char *flightNumber = _selected_flight["flight"];
            set_text("label", "label_cancel_flight_data", const_cast<char *>(flightNumber));

            const char *bay = _selected_flight["bay"];
            set_text("label", "label_cancel_flight_bay_data", const_cast<char *>(bay));

            const char *std = _selected_flight["std"];
            set_text("label", "label_cancel_flight_std_data", const_cast<char *>(std));

            const char *etd = _selected_flight["etd"];
            set_text("label", "label_cancel_flight_etd_data", const_cast<char *>(etd));

            const char *gate = _selected_flight["gate"];
            set_text("label", "label_cancel_flight_gate_data", const_cast<char *>(gate));

            set_visible("cancel_flight", "true");
            _selected_flight.clear();
        }
        else if (STONER.widget != NULL && (strcmp(STONER.widget, "button_cancel_yes") == 0))
        {
            page = 2;

            set_visible("top_overlay_status", "false");
            set_visible("bottom_overlay", "false");
            set_visible("center_overlay_confirm", "false");
            set_visible("center_overlay_standby", "false");
            set_visible("center_overlay_push", "false");
            set_visible("center_overlay_finished", "false");
            set_visible("cancel_flight", "false");
            set_visible("blank", "false");
            refreshData();

            isLogin = false;
        }
        else if (STONER.widget != NULL && (strcmp(STONER.widget, "button_cancel_no") == 0))
        {

            set_visible("cancel_flight", "false");
        }
        receive_over_flage = 0;
    }
}

void SerialLCD::page5()
{
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