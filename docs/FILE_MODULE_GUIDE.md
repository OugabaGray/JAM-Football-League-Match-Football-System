# File Management Module - Integration Guide (Section E)

Build:  g++ -std=c++17 -o kpfl main.cpp FileManager.cpp ReportGenerator.cpp <teammates' .cpp files>

## 1. Sequential files (teams, players, fixtures, results, ...)
Each class inherits Persistable and adds a static fromRecord():

    class Player : public Person /* which is Persistable */ {
    public:
        std::string serialize() const override {
            return Codec::join({id_, name_, std::to_string(age_), position_, std::to_string(jersey_)});
        }
        static Player fromRecord(const std::vector<std::string>& f) {
            Codec::requireFields(f, 5, "Player");
            return Player(f[0], f[1], Codec::toInt(f[2], "age"), f[3], Codec::toInt(f[4], "jersey"));
        }
    };

Save / load (works with vector<Player>, vector<Player*>, vector<shared_ptr<Player>>):

    FileManager::saveCollection(DataFiles::PLAYERS, players, "players");
    auto players = FileManager::loadCollection<Player>(DataFiles::PLAYERS, Player::fromRecord);

For polymorphic types (Product -> Merchandise/FoodItem), put a type tag as field 0
("MERCH" / "FOOD") and have the factory pick the subclass from it.

## 2. Random access file (products inventory, tickets)
    struct ProductRecord { int id; char name[30]; double price; int quantity; };
    RandomAccessFile<ProductRecord> inventory(DataFiles::PRODUCTS_BIN);
    size_t idx = inventory.append(rec);        // returns record number
    auto r = inventory.read(idx); r.quantity--; inventory.update(idx, r);

Use copyToFixed(rec.name, "text") to fill char arrays. Never put std::string in a record.

## 3. Others
- InputValidator: requireDate/requireTime/requireNonNegative/requireMinAge/readInt ...
- ReportGenerator: build a Report {title, headers, rows, footer}; print() or saveAsText()/saveAsCsv()
- TestManager: check/expectEqual/expectThrows/expectNoThrow, then saveReport() for evidence
- Exceptions: FileException family + ValidationException live in FileExceptions.h.
  Agree with the group on one shared exceptions header if business-rule exceptions are separate.
