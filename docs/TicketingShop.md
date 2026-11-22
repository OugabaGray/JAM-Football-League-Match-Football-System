# Ticketing and Stadium Shop Module

## Purpose
This module manages spectators, ticket sales, shop inventory,
payments, tax calculations, receipts and sales reports.

## Classes
- Spectator: Inherits from the shared Person class and represents
  a customer purchasing tickets or products.
- Ticket: Stores the ticket ID, spectator ID, fixture ID,
  category and tax-inclusive price.
- Product: Abstract superclass containing product ID, name,
  price and stock quantity.
- Merchandise: Inherits from Product and adds size and team name.
- FoodItem: Inherits from Product and identifies food or beverages.
- StadiumShop: Coordinates inventory, transactions, ticket
  capacity checks, receipts, reports and file storage.

## Object-Oriented Features
- Encapsulation: Private data is accessed through controlled methods.
- Inheritance: Spectator extends Person; Merchandise and FoodItem
  extend Product.
- Abstraction: Person and Product define abstract display behaviour.
- Polymorphism: Product subclasses display through Product
  references, and spectators display through Person references.
- Composition: StadiumShop owns its product objects using
  unique_ptr and stores ticket and receipt records.
- Exceptions: SalesError reports prohibited sales operations.

## Business Rules
- Purchase and restock quantities must be positive.
- Purchases cannot exceed available stock.
- Ticket sales cannot exceed capacity for the selected fixture.
- Product and ticket IDs must be unique within their collections.
- Payment must cover the total before a sale is completed.
- The menu requires spectator registration before purchasing.
- Prices already include 15% GCT.

## Calculations
GCT included = total × 15 / 115, rounded to the nearest cent.
Subtotal = total - GCT.
Change = payment - total.

Money is stored as whole cents to avoid floating-point errors.

## Persistence
Shop files store products, stock, fixtures, tickets and receipts.
A companion .spectators file stores spectator details.
Both files must be kept together when using menu save/load.
Loading validates both files before replacing current menu data.

## Reports
1. Completed transactions, revenue and included GCT.
2. Tickets sold and seats remaining for each fixture.
3. Products with five or fewer units remaining.

## Verified Tests
The automated demonstration passed checks for tax and change,
stock reduction, invalid purchases, duplicate ticket IDs,
stadium capacity, exact payment and shop data restoration.

Manual testing confirmed spectator registration, a jersey
purchase, and spectator restoration after restarting.

Evidence: Screenshots of the passing demonstration, purchase
receipt and restored spectator.

## Remaining Integration
Connect the sales menu to the group's main program and obtain
fixture capacities and prices from the shared stadium and
fixture classes. The standalone menu currently accepts these
details manually.

## Development Acknowledgment
AI assistance was used to develop and troubleshoot this module.