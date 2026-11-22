# Ticketing and Stadium Shop: Setup

## Requirements
- Visual Studio with Desktop development with C++ installed.
- Run commands in Developer PowerShell from the project root.
- Keep the existing data folder.

## Build the Interactive Demonstration
```powershell
cl /nologo /std:c++17 /EHsc /W4 /Iinclude tests/SalesMenuDemo.cpp src/personnel/Person.cpp src/sales/Spectator.cpp src/sales/Product.cpp src/sales/Merchandise.cpp src/sales/FoodItem.cpp src/sales/Ticket.cpp src/sales/StadiumShop.cpp src/sales/SalesMenu.cpp /Fe:SalesMenuDemo.exe
```

## Run
```powershell
.\SalesMenuDemo.exe
```

## Sample Data
- Jersey: M001, J$2,300, opening stock 5.
- Bottled water: F001, J$115, opening stock 10.
- Fixture: FIX001, capacity 100, ticket price J$1,150.
- All prices include GCT.

## Example Purchase
1. Select option 1 and register spectator S001.
2. Select option 4 and enter S001.
3. Enter product M001 and quantity 1.
4. Enter payment 2500.
5. Expected total: J$2,300. Expected change: J$200.

## Save and Reload
1. Select option 11 and enter data/shop.txt.
2. Exit using option 0 and restart the demonstration.
3. Select option 12 and enter data/shop.txt.
4. Confirm with 1.
5. Select option 2 to display restored spectators.

Keep shop.txt and shop.txt.spectators together.
Changes are saved only when option 11 is selected.
The demonstration starts with sample shop data until saved
data is loaded.

## Run Automated Sales Tests
```powershell
cl /nologo /std:c++17 /EHsc /W4 /Iinclude tests/SalesDemo.cpp src/sales/Product.cpp src/sales/Merchandise.cpp src/sales/FoodItem.cpp src/sales/Ticket.cpp src/sales/StadiumShop.cpp /Fe:SalesDemo.exe
```

```powershell
.\SalesDemo.exe
```

Expected final message: ALL DEMONSTRATION TESTS PASSED.

## Group Integration
The interactive demonstration has its own main function.
Build it separately from SalesDemo.cpp and src/main.cpp.

The group application can include sales/SalesMenu.h and call
runSalesMenu(shop) using its StadiumShop object.