#pragma once

// forward-declarations
class PageFrameMgr;

namespace os {

/**
 * Provides ACPI functionality
 */
class Acpi
{
public:
    Acpi(PageFrameMgr* pageFrameMgr);

private:
    PageFrameMgr* _pageFrameMgr;
};

}