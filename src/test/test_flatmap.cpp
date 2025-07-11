#include <cstddef>
#include <flat_map.hpp>
#include <chrono>
#include <cassert>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
using namespace fast;
using namespace std::chrono;

// static const char *filename = "map.dat";
static const string DICT[4573] = {
    "ability", "able", "about", "above", "abroad", "absence", "absolute", "absolutely",
    "absorb", "abuse", "academic", "accept", "acceptable", "access", "accident", "accompany",
    "accomplish", "according", "account", "accurate", "accuse", "achieve", "achievement", "acid",
    "acknowledge", "acquire", "across", "act", "action", "active", "activist", "activity",
    "actor", "actress", "actual", "actually", "adapt", "add", "addition", "additional",
    "address", "adequate", "adjust", "adjustment", "administration", "administrative", "administrator", "admire",
    "admission", "admit", "adolescent", "adopt", "adoption", "adult", "advance", "advanced",
    "advantage", "adventure", "advertise", "advertisement", "advertising", "advice", "advise", "adviser",
    "advocate", "affair", "affect", "afford", "afraid", "african", "after", "afternoon",
    "afterward", "again", "against", "age", "agency", "agenda", "agent", "aggressive",
    "ago", "agree", "agreement", "agricultural", "agriculture", "ahead", "aid", "aide",
    "aim", "air", "aircraft", "airline", "airport", "alarm", "album", "alcohol",
    "alive", "all", "alliance", "allow", "ally", "almost", "alone", "along",
    "already", "also", "alter", "alternative", "although", "always", "amazing", "ambassador",
    "ambition", "ambitious", "amendment", "american", "among", "amount", "analysis", "analyst",
    "analyze", "ancient", "and", "anger", "angle", "angry", "animal", "anniversary",
    "announce", "announcement", "annual", "another", "answer", "anticipate", "anxiety", "any",
    "anybody", "anymore", "anyone", "anything", "anyway", "anywhere", "apartment", "apologize",
    "apology", "apparent", "apparently", "appeal", "appear", "appearance", "apple", "application",
    "apply", "appoint", "appointment", "appreciate", "approach", "appropriate", "approval", "approve",
    "approximately", "arab", "architect", "architecture", "area", "argue", "argument", "arise",
    "arm", "armed", "army", "around", "arrange", "arrangement", "arrest", "arrival",
    "arrive", "art", "article", "artificial", "artist", "artistic", "asian", "aside",
    "ask", "asleep", "aspect", "assault", "assert", "assess", "assessment", "asset",
    "assign", "assignment", "assist", "assistance", "assistant", "associate", "association", "assume",
    "assumption", "assure", "astronomer", "athlete", "athletic", "atmosphere", "attach", "attack",
    "attempt", "attend", "attention", "attitude", "attorney", "attract", "attractive", "attribute",
    "audience", "author", "authority", "authorize", "auto", "automatic", "automatically", "automobile",
    "available", "average", "avoid", "await", "award", "aware", "awareness", "away",
    "awful", "baby", "back", "background", "bacteria", "bad", "badly", "bag",
    "bake", "balance", "balanced", "ball", "balloon", "ballot", "ban", "band",
    "bank", "banker", "banking", "bar", "bare", "barely", "barrel", "barrier",
    "base", "baseball", "basic", "basically", "basis", "basket", "basketball", "bat",
    "bath", "bathroom", "battery", "battle", "bay", "beach", "beam", "bean",
    "bear", "beard", "beat", "beautiful", "beauty", "because", "become", "bed",
    "bedroom", "beef", "beer", "before", "begin", "beginning", "behavior", "behind",
    "being", "belief", "believe", "bell", "belly", "belong", "below", "belt",
    "bench", "bend", "beneath", "benefit", "beside", "besides", "best", "bet",
    "better", "between", "beyond", "bible", "bicycle", "bid", "big", "bike",
    "bill", "billion", "bind", "biological", "biology", "bird", "birth", "birthday",
    "bishop", "bit", "bite", "bitter", "black", "blade", "blame", "blank",
    "blanket", "blast", "blend", "bless", "blessing", "blind", "block", "blonde",
    "blood", "bloody", "blow", "blue", "board", "boat", "body", "boil",
    "bold", "bolt", "bomb", "bombing", "bond", "bone", "bonus", "book",
    "boom", "boost", "boot", "border", "bore", "born", "borrow", "boss",
    "both", "bother", "bottle", "bottom", "bounce", "boundary", "bow", "bowl",
    "box", "boy", "boyfriend", "brain", "brake", "branch", "brand", "brave",
    "bread", "break", "breakfast", "breast", "breath", "breathe", "breathing", "breeze",
    "brick", "bridge", "brief", "briefly", "bright", "brilliant", "bring", "british",
    "broad", "broadcast", "broken", "broker", "bronze", "brother", "brown", "brush",
    "bubble", "buck", "bucket", "buddy", "budget", "build", "builder", "building",
    "bulb", "bulk", "bullet", "bunch", "burden", "bureau", "burn", "burning",
    "burst", "bury", "bus", "bush", "business", "businessman", "busy", "but",
    "butter", "button", "buy", "buyer", "buzz", "cabin", "cabinet", "cable",
    "cage", "cake", "calculate", "calculation", "calendar", "call", "calm", "camera",
    "camp", "campaign", "campus", "canadian", "cancer", "candidate", "candle", "candy",
    "cap", "capability", "capable", "capacity", "capital", "captain", "capture", "car",
    "carbohydrate", "carbon", "card", "care", "career", "careful", "carefully", "cargo",
    "carpet", "carrier", "carry", "cart", "cartoon", "carve", "case", "cash",
    "cast", "castle", "casual", "casualty", "cat", "catch", "category", "catholic",
    "cause", "cave", "cease", "ceiling", "celebrate", "celebration", "celebrity", "cell",
    "cellular", "cemetery", "center", "central", "century", "ceo", "ceremony", "certain",
    "certainly", "chain", "chair", "chairman", "challenge", "chamber", "champion", "championship",
    "chance", "change", "changing", "channel", "chaos", "chapter", "character", "characteristic",
    "characterize", "charge", "charity", "charm", "chart", "charter", "chase", "cheap",
    "cheat", "check", "cheek", "cheese", "chef", "chemical", "chemistry", "chest",
    "chicken", "chief", "child", "childhood", "children", "chill", "chinese", "chip",
    "chocolate", "choice", "cholesterol", "choose", "christian", "christianity", "christmas", "church",
    "cigarette", "circle", "circuit", "circumstance", "cite", "citizen", "citizenship", "city",
    "civic", "civil", "civilian", "civilization", "claim", "class", "classic", "classical",
    "classify", "classroom", "clay", "clean", "clear", "clearly", "clerk", "click",
    "client", "cliff", "climate", "climb", "clinic", "clinical", "clip", "clock",
    "close", "closed", "closely", "closer", "closest", "closet", "cloth", "clothes",
    "clothing", "cloud", "club", "clue", "cluster", "coach", "coal", "coalition",
    "coast", "coastal", "coat", "cocaine", "code", "coffee", "cognitive", "coin",
    "cold", "collapse", "collar", "colleague", "collect", "collection", "collective", "collector",
    "college", "colonial", "colony", "color", "colorful", "column", "columnist", "combat",
    "combination", "combine", "combined", "come", "comedy", "comfort", "comfortable", "comic",
    "coming", "command", "commander", "comment", "commercial", "commission", "commissioner", "commit",
    "commitment", "committee", "commodity", "common", "commonly", "communicate", "communication", "communist",
    "community", "companion", "company", "comparable", "compare", "comparison", "compel", "compelling",
    "compensation", "compete", "competition", "competitive", "competitor", "complain", "complaint", "complete",
    "completely", "complex", "complexity", "compliance", "complicated", "comply", "component", "compose",
    "composition", "compound", "comprehensive", "comprise", "compromise", "computer", "concede", "conceive",
    "concentrate", "concentration", "concept", "conception", "concern", "concerned", "concerning", "concert",
    "conclude", "conclusion", "concrete", "condemn", "condition", "conduct", "conference", "confidence",
    "confident", "confirm", "conflict", "confront", "confrontation", "confuse", "confusion", "congress",
    "congressional", "connect", "connection", "conscience", "conscious", "consciousness", "consecutive", "consensus",
    "consent", "consequence", "consequently", "conservation", "conservative", "consider", "considerable", "considerably",
    "consideration", "consist", "consistent", "consistently", "conspiracy", "constant", "constantly", "constitute",
    "constitution", "constitutional", "constraint", "construct", "construction", "consult", "consultant", "consume",
    "consumer", "consumption", "contact", "contain", "container", "contemplate", "contemporary", "contend",
    "content", "contest", "context", "continent", "continue", "continued", "continuing", "continuous",
    "contract", "contractor", "contrast", "contribute", "contribution", "contributor", "control", "controversial",
    "controversy", "convenience", "convention", "conventional", "conversation", "conversion", "convert", "convey",
    "convict", "conviction", "convince", "convinced", "cook", "cookie", "cooking", "cool",
    "cooperate", "cooperation", "cooperative", "coordinate", "coordinator", "cop", "cope", "copper",
    "copy", "cord", "core", "corn", "corner", "corporate", "corporation", "correct",
    "correctly", "correlation", "correspond", "correspondence", "correspondent", "corridor", "corrupt", "corruption",
    "cost", "costly", "costume", "cottage", "cotton", "couch", "could", "council",
    "counsel", "counseling", "counselor", "count", "counter", "counterpart", "country", "county",
    "coup", "couple", "courage", "course", "court", "courtroom", "cousin", "cover",
    "coverage", "cow", "crack", "craft", "crash", "crawl", "crazy", "cream",
    "create", "creation", "creative", "creativity", "creator", "creature", "credibility", "credit",
    "crew", "crime", "criminal", "crisis", "criteria", "critic", "critical", "criticism",
    "criticize", "critique", "crop", "cross", "crowd", "crowded", "crucial", "cruel",
    "cruise", "crush", "cry", "crystal", "cuisine", "cultural", "culture", "curious",
    "currency", "current", "currently", "curriculum", "curtain", "curve", "custody", "custom",
    "customer", "cut", "cute", "cycle", "daily", "damage", "damn", "dance",
    "dancer", "dancing", "danger", "dangerous", "dare", "dark", "darkness", "data",
    "database", "date", "daughter", "dawn", "day", "dead", "deadline", "deadly",
    "deal", "dealer", "dealing", "dear", "death", "debate", "debris", "debt",
    "debut", "decade", "decent", "decide", "decision", "deck", "declare", "decline",
    "decorate", "decrease", "dedicate", "deem", "deep", "deeply", "default", "defeat",
    "defend", "defendant", "defender", "defense", "defensive", "deferred", "deficit", "define",
    "definitely", "definition", "degree", "delay", "deliberate", "deliberately", "delicate", "delight",
    "deliver", "delivery", "demand", "democracy", "democrat", "democratic", "demographic", "demonstrate",
    "demonstration", "denial", "dense", "density", "deny", "depart", "department", "departure",
    "depend", "dependence", "dependent", "depict", "deploy", "deposit", "depressed", "depression",
    "depth", "deputy", "derive", "describe", "description", "desert", "deserve", "design",
    "designer", "desire", "desk", "desperate", "desperately", "despite", "dessert", "destination",
    "destroy", "destruction", "detail", "detailed", "detect", "detective", "determination", "determine",
    "determined", "devastate", "develop", "developer", "developing", "development", "developmental", "device",
    "devil", "devote", "diabetes", "diagnose", "diagnosis", "dialogue", "diamond", "diary",
    "dictate", "die", "diet", "dietary", "differ", "difference", "different", "differently",
    "difficult", "difficulty", "dig", "digital", "dignity", "dilemma", "dimension", "diminish",
    "dining", "dinner", "dip", "diplomat", "diplomatic", "direct", "direction", "directly",
    "director", "dirt", "dirty", "disability", "disabled", "disagree", "disagreement", "disappear",
    "disappoint", "disappointed", "disappointment", "disaster", "disc", "discharge", "discipline", "disclose",
    "discount", "discourage", "discourse", "discover", "discovery", "discrimination", "discuss", "discussion",
    "disease", "dish", "disk", "dismiss", "disorder", "display", "disposal", "dispose",
    "dispute", "disrupt", "distance", "distant", "distinct", "distinction", "distinctive", "distinguish",
    "distract", "distribute", "distribution", "district", "disturb", "disturbing", "diverse", "diversity",
    "divide", "divine", "division", "divorce", "dna", "doctor", "doctrine", "document",
    "documentary", "documentation", "dog", "doll", "dollar", "domain", "domestic", "dominant",
    "dominate", "donate", "donation", "donor", "door", "doorway", "dose", "dot",
    "double", "doubt", "down", "downtown", "dozen", "draft", "drag", "drain",
    "drama", "dramatic", "dramatically", "draw", "drawer", "drawing", "dream", "dress",
    "dried", "drift", "drill", "drink", "drinking", "drive", "driver", "driveway",
    "driving", "drop", "drown", "drug", "drum", "drunk", "dry", "duck",
    "due", "dumb", "dump", "during", "dust", "duty", "dying", "dynamic",
    "dynamics", "dynasty", "each", "eager", "ear", "early", "earn", "earning",
    "earnings", "earth", "earthquake", "ease", "easier", "easily", "east", "eastern",
    "easy", "eat", "eating", "ecological", "ecology", "economic", "economical", "economically",
    "economics", "economist", "economy", "ecosystem", "edge", "edit", "edition", "editor",
    "editorial", "educate", "education", "educational", "educator", "effect", "effective", "effectively",
    "effectiveness", "efficiency", "efficient", "effort", "egg", "ego", "eight", "eighth",
    "either", "elaborate", "elbow", "elder", "elderly", "elect", "election", "electric",
    "electrical", "electricity", "electron", "electronic", "electronics", "elegant", "element", "elementary",
    "elephant", "elevate", "elevation", "elevator", "eleven", "eligibility", "eligible", "eliminate",
    "elimination", "elite", "else", "elsewhere", "email", "embarrassed", "embarrassment", "embrace",
    "emerge", "emergency", "emerging", "emission", "emotion", "emotional", "emotionally", "emphasis",
    "emphasize", "empire", "employ", "employee", "employer", "employment", "empty", "enable",
    "enact", "encounter", "encourage", "encouraging", "end", "endless", "endorse", "endorsement",
    "enemy", "energy", "enforce", "enforcement", "engage", "engagement", "engine", "engineer",
    "engineering", "english", "enhance", "enhancement", "enjoy", "enormous", "enough", "enroll",
    "ensure", "enter", "enterprise", "entertainment", "enthusiasm", "enthusiastic", "entire", "entirely",
    "entitle", "entity", "entrance", "entry", "envelope", "environment", "environmental", "envision",
    "epidemic", "episode", "equal", "equality", "equally", "equation", "equip", "equipment",
    "equity", "equivalent", "era", "error", "escape", "escort", "especially", "essay",
    "essence", "essential", "essentially", "establish", "establishment", "estate", "estimate", "estimated",
    "ethical", "ethics", "ethnic", "european", "evaluate", "evaluation", "evangelical", "even",
    "evening", "event", "eventually", "ever", "every", "everybody", "everyday", "everyone",
    "everything", "everywhere", "evidence", "evident", "evidently", "evil", "evolution", "evolutionary",
    "evolve", "exact", "exactly", "exam", "examination", "examine", "example", "exceed",
    "excellent", "except", "exception", "exceptional", "exchange", "excited", "excitement", "exciting",
    "exclude", "exclusion", "exclusive", "exclusively", "excuse", "execute", "execution", "executive",
    "exercise", "exhaust", "exhibit", "exhibition", "exist", "existence", "existing", "exit",
    "exotic", "expand", "expansion", "expect", "expectation", "expected", "expedition", "expense",
    "expensive", "experience", "experienced", "experiment", "experimental", "expert", "expertise", "explain",
    "explanation", "explicit", "explode", "exploit", "exploration", "explore", "explorer", "explosion",
    "export", "expose", "exposure", "express", "expression", "extend", "extended", "extension",
    "extensive", "extent", "external", "extra", "extraordinary", "extreme", "extremely", "eye",
    "eyebrow", "fabric", "face", "facility", "fact", "factor", "factory", "faculty",
    "fade", "fail", "failure", "faint", "fair", "fairly", "faith", "faithful",
    "fall", "false", "fame", "familiar", "family", "famous", "fan", "fantasy",
    "far", "fare", "farm", "farmer", "fascinating", "fashion", "fast", "faster",
    "fat", "fatal", "fate", "father", "fatigue", "fault", "favor", "favorable",
    "favorite", "fear", "fearful", "feasible", "feather", "feature", "federal", "fee",
    "feed", "feedback", "feel", "feeling", "fellow", "female", "feminist", "fence",
    "festival", "fever", "few", "fewer", "fiber", "fiction", "field", "fierce",
    "fifteen", "fifth", "fifty", "fight", "fighter", "fighting", "figure", "file",
    "fill", "film", "filter", "final", "finally", "finance", "financial", "financing",
    "find", "finding", "fine", "finger", "finish", "finished", "fire", "firm",
    "firmly", "first", "fiscal", "fish", "fisherman", "fishing", "fist", "fit",
    "fitness", "five", "fix", "fixed", "flag", "flame", "flash", "flat",
    "flavor", "flesh", "flight", "float", "flood", "floor", "flour", "flow",
    "flower", "fluid", "fly", "flying", "focus", "fog", "fold", "folk",
    "follow", "following", "food", "fool", "foot", "football", "for", "forbid",
    "force", "forecast", "forehead", "foreign", "foreigner", "forest", "forever", "forget",
    "forgive", "fork", "form", "formal", "format", "formation", "former", "formerly",
    "formula", "forth", "fortunate", "fortunately", "fortune", "forty", "forum", "forward",
    "fossil", "foster", "found", "foundation", "founder", "four", "fourth", "fraction",
    "fragile", "fragment", "frame", "framework", "franchise", "frankly", "fraud", "free",
    "freedom", "freely", "freeze", "french", "frequency", "frequent", "frequently", "fresh",
    "freshman", "friend", "friendly", "friendship", "frightened", "frontier", "frown", "fruit",
    "frustrate", "frustration", "fuel", "full", "fully", "fun", "function", "functional",
    "fund", "fundamental", "funding", "funeral", "funny", "fur", "furniture", "furthermore",
    "future", "gain", "galaxy", "gallery", "game", "gang", "gap", "garage",
    "garbage", "garden", "garlic", "gas", "gasoline", "gate", "gather", "gathering",
    "gay", "gaze", "gear", "gender", "gene", "general", "generally", "generate",
    "generation", "generator", "generic", "generous", "genetic", "genius", "genre", "gentle",
    "gentleman", "gently", "genuine", "geographic", "geographical", "geography", "geological", "geology",
    "geometric", "geometry", "german", "gesture", "get", "ghost", "giant", "gift",
    "gifted", "girl", "girlfriend", "give", "given", "glad", "glance", "glass",
    "glimpse", "global", "globe", "glory", "glove", "glow", "glue", "go",
    "goal", "goat", "god", "gold", "golden", "golf", "good", "goodness",
    "goods", "gorgeous", "gospel", "gossip", "govern", "government", "governor", "grab",
    "grace", "grade", "gradual", "gradually", "graduate", "graduation", "grain", "grand",
    "grandfather", "grandmother", "grant", "grape", "grasp", "grass", "grateful", "grave",
    "gravity", "gray", "great", "greatest", "greatly", "greek", "green", "greet",
    "greeting", "grew", "grid", "grief", "grin", "grip", "grocery", "gross",
    "ground", "group", "grove", "grow", "growing", "growth", "guarantee", "guard",
    "guess", "guest", "guidance", "guide", "guideline", "guilt", "guilty", "guitar",
    "gulf", "gun", "gut", "guy", "gym", "habit", "habitat", "hair",
    "hairy", "half", "halfway", "hall", "hallway", "hand", "handful", "handle",
    "handsome", "hang", "happen", "happily", "happiness", "happy", "harassment", "harbor",
    "hard", "hardly", "hardware", "harm", "harmful", "harmony", "harsh", "harvest",
    "hat", "hate", "hatred", "haul", "have", "hay", "hazard", "head",
    "headline", "headquarters", "heal", "health", "healthcare", "healthy", "hear", "hearing",
    "heart", "heat", "heaven", "heavily", "heavy", "heel", "height", "helicopter",
    "hell", "hello", "helmet", "help", "helpful", "hence", "her", "herb",
    "here", "heritage", "hero", "heroic", "herself", "hesitate", "hey", "hi",
    "hidden", "hide", "high", "highland", "highlight", "highly", "highway", "hiking",
    "hill", "him", "himself", "hip", "hire", "hispanic", "historian", "historic",
    "historical", "historically", "history", "hit", "hockey", "hold", "holder", "holding",
    "hole", "holiday", "hollow", "holy", "home", "homeland", "homeless", "homework",
    "honest", "honestly", "honey", "honor", "hook", "hope", "hopefully", "horizon",
    "hormone", "horn", "horrible", "horror", "horse", "hospital", "host", "hostage",
    "hostile", "hostility", "hot", "hotel", "hour", "house", "household", "housing",
    "how", "however", "hug", "huge", "hull", "human", "humanitarian", "humanity",
    "humble", "humor", "hundred", "hunger", "hungry", "hunt", "hunter", "hunting",
    "hurricane", "hurry", "hurt", "husband", "hut", "hydrogen", "hypothesis", "i",
    "ice", "icon", "idea", "ideal", "identical", "identification", "identify", "identity",
    "ideological", "ideology", "if", "ignorance", "ignorant", "ignore", "ill", "illegal",
    "illegally", "illness", "illusion", "illustrate", "illustration", "image", "imagery", "imagination",
    "imaginative", "imagine", "immigrant", "immigration", "immune", "impact", "impair", "imperial",
    "implement", "implementation", "implication", "imply", "import", "importance", "important", "impose",
    "impossible", "impress", "impression", "impressive", "improve", "improved", "improvement", "impulse",
    "in", "incentive", "inch", "incidence", "incident", "include", "including", "income",
    "incorporate", "increase", "increased", "increasing", "increasingly", "incredible", "incredibly", "indeed",
    "independence", "independent", "index", "indian", "indicate", "indication", "indicator", "indigenous",
    "individual", "industrial", "industry", "ineffective", "inequality", "inevitable", "inevitably", "infant",
    "infection", "infectious", "inference", "inflation", "inflict", "influence", "influential", "inform",
    "informal", "information", "infrastructure", "ingredient", "inherent", "inherit", "initial", "initially",
    "initiate", "initiative", "injure", "injured", "injury", "inmate", "inner", "innocent",
    "innovation", "innovative", "input", "inquiry", "insect", "insert", "inside", "insight",
    "insist", "inspection", "inspector", "inspiration", "inspire", "install", "installation", "instance",
    "instant", "instantly", "instead", "instinct", "institution", "institutional", "instruct", "instruction",
    "instructor", "instrument", "instrumental", "insufficient", "insult", "insurance", "intact", "intake",
    "integral", "integrate", "integrated", "integration", "integrity", "intellectual", "intelligence", "intelligent",
    "intend", "intense", "intensity", "intensive", "intent", "intention", "interact", "interaction",
    "interest", "interested", "interesting", "interfere", "interference", "interim", "interior", "intermediate",
    "internal", "international", "internet", "interpret", "interpretation", "interrupt", "interval", "intervention",
    "interview", "intimate", "intimidate", "into", "intriguing", "introduce", "introduction", "invade",
    "invasion", "invent", "invention", "inventory", "invest", "investigate", "investigation", "investigator",
    "investment", "investor", "invisible", "invitation", "invite", "involve", "involved", "involvement",
    "iraqi", "irish", "iron", "irony", "irrelevant", "irritate", "islamic", "island",
    "isolate", "isolated", "isolation", "israeli", "issue", "it", "italian", "item",
    "its", "itself", "jacket", "jail", "japanese", "jar", "jaw", "jazz",
    "jealous", "jeans", "jeep", "jew", "jewelry", "jewish", "job", "join",
    "joint", "joke", "journal", "journalism", "journalist", "journey", "joy", "judge",
    "judgment", "judicial", "juice", "jump", "junction", "jungle", "junior", "jurisdiction",
    "juror", "jury", "just", "justice", "justification", "justify", "keep", "keeper",
    "keeping", "key", "keyboard", "kick", "kid", "kidnap", "kidney", "kill",
    "killer", "killing", "kilogram", "kilometer", "kind", "king", "kingdom", "kiss",
    "kit", "kitchen", "knee", "knife", "knight", "knock", "knot", "know",
    "knowledge", "known", "korean", "lab", "label", "labor", "laboratory", "lack",
    "ladder", "lady", "lake", "lamb", "lamp", "land", "landing", "landmark",
    "landscape", "lane", "language", "lap", "large", "largely", "laser", "last",
    "late", "lately", "later", "latest", "latin", "latter", "laugh", "laughter",
    "launch", "laundry", "law", "lawn", "lawsuit", "lawyer", "lay", "layer",
    "lead", "leader", "leadership", "leading", "leaf", "league", "lean", "leap",
    "learn", "learning", "least", "leather", "leave", "lecture", "left", "leg",
    "legacy", "legal", "legally", "legend", "legislation", "legislative", "legislator", "legislature",
    "legitimate", "lemon", "lend", "length", "lens", "less", "lesser", "lesson",
    "let", "letter", "level", "liability", "liable", "liberal", "liberty", "library",
    "license", "lid", "lie", "life", "lifestyle", "lifetime", "lift", "light",
    "lighting", "lightning", "like", "likely", "likewise", "limb", "limit", "limitation",
    "limited", "line", "linear", "linger", "linguistic", "link", "lion", "lip",
    "liquid", "list", "listen", "listener", "literally", "literary", "literature", "little",
    "liturgical", "live", "lively", "liver", "living", "load", "loan", "lobby",
    "local", "locally", "locate", "location", "lock", "locker", "log", "logic",
    "logical", "lonely", "long", "longtime", "look", "loop", "loose", "lord",
    "lose", "loss", "lost", "lot", "loud", "love", "lovely", "lover",
    "low", "lower", "loyal", "loyalty", "luck", "lucky", "lunch", "lung",
    "luxury", "machine", "mad", "magazine", "magic", "magical", "magnet", "magnetic",
    "magnificent", "magnitude", "mail", "main", "mainland", "mainly", "mainstream", "maintain",
    "maintenance", "major", "majority", "make", "maker", "makeup", "making", "male",
    "mall", "man", "manage", "management", "manager", "managing", "mandate", "mandatory",
    "manipulate", "manipulation", "manner", "mansion", "manual", "manufacture", "manufacturer", "manufacturing",
    "many", "map", "marble", "march", "margin", "marginal", "marine", "mark",
    "marker", "market", "marketing", "marketplace", "marriage", "married", "marry", "mask",
    "mass", "massive", "master", "match", "mate", "material", "mathematical", "mathematics",
    "matrix", "matter", "maximum", "may", "maybe", "mayor", "me", "meadow",
    "meal", "mean", "meaning", "meaningful", "meantime", "meanwhile", "measure", "measurement",
    "meat", "mechanic", "mechanical", "mechanism", "medal", "media", "medical", "medication",
    "medicine", "medieval", "medium", "meet", "meeting", "melt", "member", "membership",
    "memorial", "memory", "men", "mental", "mentally", "mention", "mentor", "menu",
    "merchant", "mercy", "mere", "merely", "merge", "merger", "merit", "mess",
    "message", "metal", "metaphor", "meter", "method", "metropolitan", "mexican", "mid",
    "middle", "midnight", "midst", "might", "migration", "mild", "military", "milk",
    "mill", "million", "mind", "mine", "mineral", "minimal", "minimize", "minimum",
    "minister", "ministry", "minor", "minority", "minute", "miracle", "mirror", "miss",
    "missile", "mission", "missionary", "mistake", "mix", "mixture", "mobile", "mode",
    "model", "moderate", "moderately", "modern", "modest", "modification", "modify", "module",
    "mom", "moment", "momentum", "monarch", "monday", "monetary", "money", "monitor",
    "monk", "monkey", "monopoly", "monster", "month", "monthly", "monument", "mood",
    "moon", "moral", "morality", "morally", "more", "moreover", "morning", "mortality",
    "mortgage", "mosque", "most", "mostly", "mother", "motion", "motivate", "motivation",
    "motive", "motor", "motorcycle", "mount", "mountain", "mounting", "mourn", "mouth",
    "move", "movement", "movie", "moving", "multiple", "municipal", "murder", "muscle",
    "museum", "mushroom", "music", "musical", "musician", "muslim", "must", "mute",
    "mutual", "my", "myself", "mysterious", "mystery", "myth", "nail", "naked",
    "name", "narrative", "narrow", "nasty", "nation", "national", "nationwide", "native",
    "natural", "naturally", "nature", "naval", "near", "nearby", "nearly", "neat",
    "necessarily", "necessary", "necessity", "neck", "need", "negative", "neglect", "negotiate",
    "negotiation", "negotiator", "neighbor", "neighborhood", "neighboring", "neither", "nerve", "nervous",
    "nest", "net", "network", "neutral", "never", "nevertheless", "new", "newly",
    "news", "newspaper", "next", "nice", "niche", "night", "nightmare", "nine",
    "nineteenth", "ninth", "no", "noble", "nobody", "nod", "noise", "noisy",
    "nomination", "nominee", "none", "nonetheless", "nonprofit", "nonsense", "noon", "nor",
    "norm", "normal", "normally", "north", "northeast", "northern", "northwest", "nose",
    "not", "note", "notebook", "nothing", "notice", "notion", "novel", "now",
    "nowhere", "nuclear", "number", "numerous", "nurse", "nut", "nutrient", "nutrition",
    "oak", "oath", "obey", "object", "objection", "objective", "obligation", "observation",
    "observe", "observer", "obsession", "obstacle", "obtain", "obvious", "obviously", "occasion",
    "occasional", "occasionally", "occupation", "occupy", "occur", "ocean", "odd", "odds",
    "of", "off", "offence", "offend", "offensive", "offer", "offering", "office",
    "officer", "official", "officially", "often", "oh", "oil", "ok", "okay",
    "old", "oldest", "olive", "olympic", "olympics", "omit", "on", "once",
    "one", "ongoing", "onion", "online", "only", "onset", "onto", "open",
    "opening", "openly", "opera", "operate", "operating", "operation", "operator", "opinion",
    "opponent", "opportunity", "oppose", "opposed", "opposite", "opposition", "opt", "optical",
    "optimistic", "optimization", "optimize", "option", "optional", "or", "oral", "orange",
    "orbit", "orchestra", "order", "ordinary", "organic", "organism", "organization", "organizational",
    "organize", "organized", "orientation", "origin", "original", "originally", "originate", "other",
    "otherwise", "ought", "our", "ourselves", "out", "outcome", "outdoor", "outer",
    "outlet", "outline", "outlook", "output", "outside", "outsider", "outstanding", "oven",
    "over", "overall", "overcome", "overhead", "overlook", "overnight", "overseas", "oversee",
    "overwhelm", "overwhelming", "owe", "own", "owner", "ownership", "oxygen", "pace",
    "pack", "package", "pad", "page", "pain", "painful", "paint", "painter",
    "painting", "pair", "palace", "pale", "palm", "pan", "panel", "panic",
    "pant", "paper", "par", "parade", "parent", "parental", "parish", "park",
    "parking", "parliament", "part", "partial", "partially", "participant", "participate", "participation",
    "particle", "particular", "particularly", "partly", "partner", "partnership", "party", "pass",
    "passage", "passenger", "passing", "passion", "passport", "past", "pasta", "pastor",
    "patch", "patent", "path", "patience", "patient", "patrol", "patron", "pattern",
    "pause", "pay", "payment", "payroll", "peace", "peaceful", "peak", "peanut",
    "peasant", "peel", "peer", "pen", "penalty", "pencil", "pension", "people",
    "pepper", "per", "perceive", "percentage", "perception", "perfect", "perfectly", "perform",
    "performance", "performer", "perhaps", "period", "periodic", "permanent", "permanently", "permission",
    "permit", "persecution", "persist", "persistent", "person", "personal", "personality", "personally",
    "personnel", "perspective", "persuade", "pest", "pet", "petition", "petroleum", "phase",
    "phenomenon", "philosopher", "philosophical", "philosophy", "phone", "photo", "photograph", "photographer",
    "photography", "phrase", "physical", "physically", "physician", "physics", "piano", "pick",
    "pickup", "picture", "pie", "piece", "pier", "pig", "pile", "pill",
    "pillow", "pilot", "pin", "pine", "pink", "pioneer", "pipe", "pit",
    "pitch", "pitcher", "pizza", "place", "placement", "plain", "plaintiff", "plan",
    "plane", "planet", "planner", "planning", "plant", "plantation", "planter", "plasma",
    "plastic", "plate", "platform", "plausible", "play", "player", "playoff", "plea",
    "plead", "pleasant", "please", "pleased", "pleasure", "plenty", "plot", "plunge",
    "plus", "plywood", "pocket", "poem", "poet", "poetry", "point", "poison",
    "poisonous", "poke", "polar", "pole", "police", "policeman", "policy", "political",
    "politically", "politician", "politics", "poll", "pollution", "pond", "pool", "poor",
    "pop", "popular", "popularity", "population", "porch", "pork", "port", "portfolio",
    "portion", "portrait", "portray", "pose", "position", "positive", "possess", "possession",
    "possibility", "possible", "possibly", "post", "poster", "pot", "potato", "potential",
    "potentially", "pound", "pour", "poverty", "powder", "power", "powerful", "practical",
    "practically", "practice", "practitioner", "praise", "pray", "prayer", "preach", "preacher",
    "precedent", "precious", "precise", "precisely", "precision", "predator", "predecessor", "predict",
    "prediction", "predominant", "prefer", "preference", "preferential", "pregnant", "preliminary", "premier",
    "premise", "premium", "preparation", "prepare", "prepared", "prescription", "presence", "present",
    "presentation", "preservation", "preserve", "preside", "presidency", "president", "presidential", "press",
    "pressure", "presumably", "presume", "pretend", "pretty", "prevail", "prevalence", "prevalent",
    "prevent", "prevention", "previous", "previously", "prey", "price", "pride", "priest",
    "primarily", "primary", "prime", "principal", "principle", "print", "printer", "prior",
    "priority", "prison", "prisoner", "privacy", "private", "privately", "privilege", "privileged",
    "prize", "pro", "probability", "probable", "probably", "probe", "problem", "problematic",
    "procedural", "procedure", "proceed", "proceeding", "process", "processing", "processor", "proclaim",
    "produce", "producer", "product", "production", "productive", "productivity", "profession", "professional",
    "professor", "profile", "profit", "profitable", "profound", "profoundly", "program", "programming",
    "progress", "progressive", "prohibit", "project", "projection", "prominent", "promise", "promising",
    "promote", "promotion", "prompt", "proof", "proper", "properly", "property", "proportion",
    "proportional", "proposal", "propose", "proposed", "proposition", "prosecute", "prosecution", "prosecutor",
    "prospect", "prospective", "protect", "protection", "protective", "protein", "protest", "protestant",
    "protocol", "proud", "proudly", "prove", "proven", "provide", "provided", "provider",
    "province", "provincial", "provision", "provocative", "provoke", "psychological", "psychologist", "psychology",
    "public", "publication", "publicity", "publicly", "publish", "publisher", "publishing", "pull",
    "pulse", "pump", "punch", "punishment", "pupil", "purchase", "pure", "purely",
    "purple", "purpose", "purse", "pursue", "pursuit", "push", "put", "puzzle",
    "pyramid", "qualification", "qualified", "qualify", "quality", "quantity", "quarter", "quarterback",
    "queen", "quest", "question", "questionnaire", "queue", "quick", "quickly", "quiet",
    "quietly", "quit", "quite", "quota", "quote", "rabbit", "race", "racial",
    "racism", "rack", "radar", "radiation", "radical", "radio", "rage", "raid",
    "rail", "railroad", "railway", "rain", "rainbow", "raise", "rally", "ram",
    "random", "randomly", "range", "rank", "ranking", "rape", "rapid", "rapidly",
    "rare", "rarely", "rat", "rate", "rather", "rating", "ratio", "rational",
    "raw", "ray", "reach", "react", "reaction", "read", "reader", "readily",
    "reading", "ready", "real", "realistic", "reality", "realize", "really", "realm",
    "reap", "rear", "reason", "reasonable", "reasonably", "reasoning", "rebel", "rebellion",
    "rebuild", "recall", "receipt", "receive", "receiver", "recent", "recently", "reception",
    "recession", "recipe", "recipient", "recognition", "recognize", "recommend", "recommendation", "reconcile",
    "reconciliation", "record", "recording", "recover", "recovery", "recruit", "recruitment", "recycle",
    "red", "reduce", "reduction", "redundant", "refer", "reference", "referendum", "reflect",
    "reflection", "reform", "refugee", "refuse", "regard", "regarding", "regardless", "regime",
    "region", "regional", "register", "regret", "regular", "regularly", "regulate", "regulation",
    "regulatory", "rehabilitation", "reinforce", "reject", "rejection", "relate", "related", "relation",
    "relationship", "relative", "relatively", "relax", "relaxed", "release", "relevant", "reliability",
    "reliable", "relief", "relieve", "religion", "religious", "reluctant", "rely", "remain",
    "remainder", "remaining", "remains", "remark", "remarkable", "remarkably", "remedy", "remember",
    "remind", "reminder", "remote", "removal", "remove", "renaissance", "render", "renew",
    "renewable", "rent", "rental", "repair", "repeat", "repeated", "repeatedly", "replace",
    "replacement", "replay", "reply", "report", "reportedly", "reporter", "reporting", "represent",
    "representation", "representative", "republic", "republican", "reputation", "request", "require", "required",
    "requirement", "rescue", "research", "researcher", "resemble", "reservation", "reserve", "reserved",
    "reservoir", "residence", "resident", "residential", "residual", "resign", "resignation", "resist",
    "resistance", "resistant", "resolution", "resolve", "resort", "resource", "respect", "respectively",
    "respond", "respondent", "response", "responsibility", "responsible", "rest", "restaurant", "restoration",
    "restore", "restraint", "restrict", "restriction", "restrictive", "result", "resume", "retail",
    "retailer", "retain", "retention", "retire", "retired", "retirement", "retreat", "retrieve",
    "return", "reveal", "revelation", "revenge", "revenue", "reverse", "review", "revolution",
    "revolutionary", "reward", "rhetoric", "rhythm", "rice", "rich", "rid", "ride",
    "rider", "ridge", "ridiculous", "rifle", "right", "rim", "ring", "riot",
    "rip", "rise", "risk", "risky", "ritual", "rival", "rivalry", "river",
    "road", "roast", "rob", "robbery", "robot", "rock", "rocket", "rod",
    "role", "roll", "rolling", "roman", "romance", "romantic", "roof", "room",
    "root", "rope", "rose", "roster", "rotate", "rotation", "rough", "roughly",
    "round", "route", "routine", "routinely", "row", "royal", "rub", "rubber",
    "ruin", "rule", "ruling", "rumor", "run", "runner", "running", "rural",
    "rush", "russian", "sacred", "sacrifice", "sad", "safe", "safely", "safety",
    "sail", "sailing", "sailor", "salad", "salary", "sale", "sales", "salmon",
    "salt", "salvation", "same", "sample", "sanction", "sand", "sandwich", "satellite",
    "satisfaction", "satisfactory", "satisfy", "sauce", "save", "saving", "savings", "say",
    "scale", "scan", "scandal", "scare", "scared", "scary", "scatter", "scenario",
    "scene", "scenery", "schedule", "scheme", "scholar", "scholarship", "school", "science",
    "scientific", "scientist", "scope", "score", "scramble", "scratch", "scream", "screen",
    "screening", "script", "sculpture", "sea", "seal", "search", "season", "seat",
    "secondary", "secret", "secretary", "section", "sector", "secular", "secure", "security",
    "see", "seed", "seek", "seem", "seemingly", "segment", "seize", "seldom",
    "select", "selected", "selection", "selective", "self", "sell", "seller", "selling",
    "semester", "seminar", "senate", "senator", "send", "senior", "sensation", "sense",
    "sensitive", "sensitivity", "sensor", "sensory", "sentence", "sentiment", "separate", "separation",
    "sequence", "serial", "series", "serious", "seriously", "sermon", "servant", "serve",
    "server", "service", "session", "set", "setting", "settle", "settlement", "seven",
    "seventeen", "seventh", "seventy", "several", "severe", "severely", "sex", "sexual",
    "sexuality", "sexually", "shade", "shadow", "shake", "shall", "shallow", "shame",
    "shape", "share", "shared", "shareholder", "shark", "sharp", "sharply", "shatter",
    "she", "shed", "sheep", "sheet", "shelf", "shell", "shelter", "shift",
    "shine", "shiny", "ship", "shirt", "shock", "shocked", "shocking", "shoe",
    "shoot", "shooting", "shop", "shopping", "shore", "short", "shortage", "shortly",
    "shorts", "shot", "should", "shoulder", "shout", "show", "shower", "showing",
    "shrink", "shrug", "shut", "shuttle", "shy", "sibling", "sick", "side",
    "sidewalk", "siege", "sigh", "sight", "sign", "signal", "signature", "significance",
    "significant", "significantly", "silence", "silent", "silk", "silly", "silver", "similar",
    "similarity", "similarly", "simple", "simply", "sin", "since", "sing", "singer",
    "singing", "single", "sink", "sir", "sister", "sit", "site", "situation",
    "six", "sixteen", "sixth", "sixty", "size", "ski", "skill", "skilled",
    "skin", "skip", "skirt", "skull", "sky", "slam", "slap", "slave",
    "slavery", "sleep", "sleeve", "slice", "slide", "slight", "slightly", "slim",
    "slip", "slope", "slot", "slow", "slowly", "small", "smart", "smell",
    "smile", "smoke", "smooth", "snake", "snap", "snow", "so", "soap",
    "soccer", "social", "socially", "society", "sociology", "sock", "soft", "software",
    "soil", "solar", "soldier", "sole", "solely", "solid", "solidarity", "solution",
    "solve", "some", "somebody", "somehow", "someone", "something", "sometime", "sometimes",
    "somewhat", "somewhere", "son", "song", "soon", "sophisticated", "sorry", "sort",
    "soul", "sound", "soup", "source", "south", "southeast", "southern", "southwest",
    "soviet", "space", "spanish", "spare", "spark", "speak", "speaker", "special",
    "specialist", "specialize", "specialized", "specialty", "species", "specific", "specifically", "specification",
    "specify", "specimen", "spectacle", "spectacular", "spectator", "spectrum", "speculate", "speculation",
    "speech", "speed", "spell", "spelling", "spend", "spending", "sphere", "spice",
    "spider", "spin", "spine", "spiral", "spirit", "spiritual", "spirituality", "spit",
    "spite", "split", "sponsor", "spontaneous", "spoon", "sport", "spot", "spouse",
    "spray", "spread", "spring", "sprinkle", "spy", "squad", "square", "squeeze",
    "stability", "stable", "stack", "stadium", "staff", "stage", "stair", "stake",
    "stance", "stand", "standard", "standing", "star", "stare", "start", "starter",
    "starting", "state", "statement", "station", "statistical", "statistics", "statue", "status",
    "statutory", "stay", "steady", "steal", "steam", "steel", "steep", "steer",
    "stem", "step", "stereotype", "sterile", "stick", "sticky", "stiff", "still",
    "stimulate", "stimulus", "sting", "stir", "stock", "stomach", "stone", "stop",
    "storage", "store", "storm", "story", "stove", "straight", "straighten", "strain",
    "strange", "stranger", "strategic", "strategy", "straw", "strawberry", "stream", "street",
    "strength", "strengthen", "stress", "stretch", "strict", "strictly", "stride", "strike",
    "striking", "string", "strip", "stripe", "strive", "stroke", "strong", "strongly",
    "structural", "structure", "struggle", "student", "studio", "study", "stuff", "stumble",
    "stun", "stunning", "stupid", "style", "subject", "subjective", "submit", "subsequent",
    "subsequently", "subsidy", "substance", "substantial", "substantially", "substitute", "subtitle", "subtle",
    "suburb", "suburban", "succeed", "success", "successful", "successfully", "such", "suck",
    "sudden", "suddenly", "sue", "suffer", "suffering", "sufficient", "sufficiently", "sugar",
    "suggest", "suggestion", "suicide", "suit", "suitable", "suite", "sum", "summarize",
    "summary", "summer", "summit", "sun", "sunday", "sunrise", "sunset", "sunshine",
    "super", "superb", "superior", "supermarket", "supervisor", "supplier", "supply", "support",
    "supporter", "supporting", "supportive", "suppose", "supposed", "supposedly", "supreme", "sure",
    "surely", "surface", "surgeon", "surgery", "surgical", "surprise", "surprised", "surprising",
    "surprisingly", "surrender", "surround", "surrounding", "surveillance", "survey", "survival", "survive",
    "survivor", "suspect", "suspend", "suspension", "suspicion", "suspicious", "sustain", "sustainable",
    "swallow", "swear", "sweater", "sweep", "sweet", "swell", "swift", "swim",
    "swimming", "swing", "switch", "sword", "symbol", "symbolic", "sympathy", "symptom",
    "syndrome", "synthesis", "system", "systematic", "systematically", "table", "tablet", "tackle",
    "tactic", "tag", "tail", "take", "tale", "talent", "talented", "talk",
    "tall", "tank", "tap", "tape", "target", "task", "taste", "tax",
    "taxpayer", "tea", "teach", "teacher", "teaching", "team", "teammate", "tear",
    "technical", "technician", "technique", "technological", "technology", "teen", "teenage", "teenager",
    "teeth", "telephone", "telescope", "television", "tell", "temperature", "temple", "temporary",
    "ten", "tenant", "tend", "tendency", "tender", "tennis", "tension", "tent",
    "term", "terminal", "terminate", "termination", "terrain", "terrible", "terribly", "terrific",
    "terrify", "territory", "terror", "terrorism", "terrorist", "test", "testify", "testimony",
    "testing", "text", "textbook", "textile", "texture", "than", "thank", "thanks",
    "thanksgiving", "that", "theater", "theatrical", "theft", "their", "them", "theme",
    "themselves", "then", "theological", "theology", "theoretical", "theory", "therapist", "therapy",
    "there", "thereafter", "thereby", "therefore", "thermal", "these", "thesis", "they",
    "thick", "thief", "thin", "thing", "think", "thinking", "third", "thirty",
    "this", "thoroughly", "those", "though", "thought", "thoughtful", "thousand", "thread",
    "threat", "threaten", "threatening", "three", "threshold", "thrill", "thrive", "throat",
    "through", "throughout", "throw", "thumb", "thunder", "thursday", "thus", "ticket",
    "tide", "tie", "tight", "tightly", "tile", "till", "timber", "time",
    "timing", "tin", "tiny", "tip", "tire", "tired", "tissue", "title",
    "to", "tobacco", "today", "toe", "together", "toilet", "tolerance", "tolerate",
    "toll", "tomato", "tomorrow", "ton", "tone", "tongue", "tonight", "too",
    "tool", "tooth", "top", "topic", "toss", "total", "totally", "touch",
    "touchdown", "tough", "tour", "tourism", "tourist", "tournament", "toward", "towards",
    "tower", "town", "toxic", "toy", "trace", "track", "trade", "trademark",
    "trading", "tradition", "traditional", "traditionally", "traffic", "tragedy", "tragic", "trail",
    "trailer", "train", "trainer", "training", "trait", "transaction", "transcend", "transcription",
    "transfer", "transform", "transformation", "transit", "transition", "translate", "translation", "transmission",
    "transmit", "transport", "transportation", "trap", "trash", "trauma", "travel", "traveler",
    "tray", "treasure", "treat", "treatment", "treaty", "tree", "tremendous", "trend",
    "trial", "tribal", "tribe", "trick", "trigger", "trillion", "trip", "triumph",
    "troop", "trophy", "tropical", "trouble", "troubled", "trouser", "truck", "true",
    "truly", "trunk", "trust", "trustee", "truth", "try", "tube", "tuck",
    "tuesday", "tuition", "tumor", "tune", "tunnel", "turkey", "turn", "turning",
    "turnover", "tv", "twelve", "twentieth", "twenty", "twice", "twin", "twist",
    "two", "type", "typical", "typically", "ugly", "ultimate", "ultimately", "umbrella",
    "unable", "uncertain", "uncertainty", "uncle", "uncomfortable", "unconscious", "under", "undergo",
    "underground", "underlying", "undermine", "understand", "understanding", "undertake", "underwater", "underwear",
    "undo", "unemployment", "unexpected", "unexpectedly", "unfair", "unfold", "unfortunate", "unfortunately",
    "unhappy", "uniform", "unify", "unique", "unit", "unite", "united", "unity",
    "universal", "universe", "university", "unknown", "unless", "unlike", "unlikely", "unlimited",
    "unprecedented", "unreasonable", "unrelated", "unstable", "unsuccessful", "until", "unusual", "unwilling",
    "up", "update", "upgrade", "upheld", "upon", "upper", "upset", "upstairs",
    "upward", "urban", "urge", "urgent", "us", "use", "used", "useful",
    "user", "usual", "usually", "utility", "utilize", "vacation", "vaccine", "vacuum",
    "vague", "valid", "validity", "valley", "valuable", "value", "van", "variable",
    "variance", "variation", "varied", "variety", "various", "vary", "vast", "vegetable",
    "vegetation", "vehicle", "velocity", "velvet", "vendor", "venture", "verbal", "verdict",
    "verify", "verse", "version", "versus", "vertical", "very", "vessel", "veteran",
    "veto", "via", "viable", "vibrant", "vice", "victim", "victory", "video",
    "view", "viewer", "village", "villain", "violate", "violation", "violence", "violent",
    "violently", "virtual", "virtually", "virtue", "virus", "visa", "visible", "vision",
    "visit", "visitor", "visual", "vital", "vitamin", "vocabulary", "vocal", "voice",
    "volume", "voluntary", "volunteer", "vote", "voter", "voting", "vow", "voyage",
    "vulnerable", "wage", "wagon", "wait", "waiter", "wake", "walk", "walking",
    "wall", "wander", "want", "war", "warehouse", "warm", "warmth", "warn",
    "warning", "warrant", "warranty", "warrior", "wash", "waste", "watch", "water",
    "wave", "way", "we", "weak", "weaken", "weakness", "wealth", "wealthy",
    "weapon", "wear", "weather", "weave", "web", "website", "wedding", "wednesday",
    "weed", "week", "weekend", "weekly", "weigh", "weight", "weird", "welcome",
    "welfare", "well", "west", "western", "wet", "whale", "what", "whatever",
    "wheat", "wheel", "when", "whenever", "where", "whereas", "wherever", "whether",
    "which", "while", "whip", "whisper", "white", "who", "whoever", "whole",
    "whom", "whose", "why", "wicked", "wide", "widely", "widespread", "widow",
    "width", "wife", "wild", "wilderness", "wildlife", "will", "willing", "willingness",
    "win", "wind", "window", "wine", "wing", "winner", "winning", "winter",
    "wipe", "wire", "wisdom", "wise", "wish", "with", "withdraw", "withdrawal",
    "within", "without", "witness", "wizard", "woman", "wonder", "wonderful", "wood",
    "wooden", "wool", "word", "work", "worker", "workforce", "working", "workout",
    "workplace", "works", "workshop", "world", "worldwide", "worm", "worried", "worry",
    "worse", "worship", "worst", "worth", "worthwhile", "worthy", "would", "wound",
    "wrap", "wreck", "wrist", "write", "writer", "writing", "written", "wrong",
    "wrongly", "yard", "yarn", "yeah", "year", "yell", "yellow", "yes",
    "yesterday", "yet", "yield", "yogurt", "you", "young", "youngster", "your",
    "yours", "yourself", "youth", "zone", "zoo"
};
constexpr size_t DICT_SIZE = sizeof(DICT) / sizeof(DICT[0]);

void test_flat_map() {
  const char* filename = "map.dat";
  
  auto start = high_resolution_clock::now();
  
  flat_map<string, int> map;
  
  int fd = open(filename, O_RDONLY);
  if (fd >= 0) {
    bool load_success = map.load(fd);
    close(fd);
    std::cout << "Loading existing map: " << (load_success ? "success" : "failed") << "\n";
    std::cout << "Loaded entries: " << map.count() << "\n";
    std::cout << "Verifying entries...\n";
    size_t verified = 0;
    size_t errors = 0;
    for (size_t i = 0; i < DICT_SIZE; i += 50) {  // Check every 50th entry
      int* value = map.find(DICT[i]);
      int expected = (i % 2 == 0);
      if (!value) {
        std::cout << "Error: Missing entry for " << DICT[i].begin() << "\n";
        errors++;
      } else if (*value != expected) {
        std::cout << "Error: Wrong value for " << DICT[i].begin() 
          << ", got '" << *value << "', expected '" << expected
          << "' iter " << i << '\n';
        errors++;
      }
      verified++;
    }
    std::cout << "Verified " << verified << " entries with " << errors << " errors\n";
    std::cout << DICT_SIZE << '\n';
  } else {
    std::cout << "No existing map found, will create new one\n";
    // Fill map with dictionary words
    std::cout << "Filling map with dictionary words...\n";
    auto fill_start = high_resolution_clock::now();
    for (size_t i = 0; i < DICT_SIZE; ++i) {
      map.insert(DICT[i], i % 2 == 0);
      if (i % 50 == 0) {
        std::cout << DICT[i].begin() << ": " << (i % 2 == 0) << '\n';
        std::cout << *map.find(DICT[i]) << '\n';
      }

    }
    auto fill_end = high_resolution_clock::now();
    std::cout << "Filled with " << map.count() << " entries in " 
      << duration_cast<microseconds>(fill_end - fill_start).count() << " μs\n";
  }
  // Save map to file
  std::cout << "Saving map...\n";
  auto save_start = high_resolution_clock::now();
  fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd < 0) {
    std::cout << "Failed to open file for writing\n";
    return;
  }
  bool save_success = map.save(fd);
  close(fd);
  auto save_end = high_resolution_clock::now();
  std::cout << "Save " << (save_success ? "successful" : "failed") 
            << " in " << duration_cast<microseconds>(save_end - save_start).count() << " μs\n";
  
  //map.print();
  
  // auto load_start = high_resolution_clock::now();
  // fd = open(filename, O_RDONLY);
  // if (fd < 0) {
    // std::cout << "Failed to open file for reading\n";
    // return;
  // }
  // bool reload_success = map.load(fd);
  // close(fd);
  // auto load_end = high_resolution_clock::now();
  // std::cout << "Reload " << (reload_success ? "successful" : "failed") 
            // << " in " << duration_cast<microseconds>(load_end - load_start).count() << " μs\n";
  // std::cout << "Map size after reload: " << map.count() << "\n";
  
  // Verify some entries
  
  // Report total time
  auto end = high_resolution_clock::now();
  std::cout << "Total test time: " << duration_cast<milliseconds>(end - start).count() << " ms\n";
}

void test_save_load() {
  flat_map<string, int> map;

  const int INSERTIONS = 1000;
  for (int i = 0; i < INSERTIONS; ++i) {
    map.insert(DICT[i], i);
  }

  for (int i = 0; i < INSERTIONS; ++i) {
    int* value = map.find(DICT[i]);
    assert(value != nullptr);
    assert(*value == i);
  }

  const char* filename = "test_save_load.dat";

  int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  assert(fd >= 0);
  assert(map.save(fd));
  close(fd);

  flat_map<string, int> map2;


  fd = open(filename, O_RDONLY);
  assert(fd >= 0);
  map2.load(fd);
  close(fd);

  static constexpr uint8_t EMPTY = 0x80;
  static constexpr uint8_t DELETED = 0xFE;

  for (size_t i = 0; i < map.cap; ++i) {
    if (map.meta[i] == EMPTY || map.meta[i] == DELETED) {
      continue;
    }
    assert(map.meta[i] == map2.meta[i]);
    std::cout << "map key at " << i << ": " << map.data[i].key.begin() << '\n';
    std::cout << "map2 key at " << i << ": " << map2.data[i].key.begin() << '\n';
    assert(map.data[i].key == map2.data[i].key);
    std::cout << "map value at " << i << ": " << map.data[i].value << '\n';
    std::cout << "map2 value at " << i << ": " << map2.data[i].value << '\n';
    assert(map.data[i].value == map2.data[i].value);
  }

  for (int i = 0; i < INSERTIONS; ++i) {
    int* value = map2.find(DICT[i]);
    assert(value != nullptr);
    assert(*value == i);
  }
}

void test_basic() {
  flat_map<string, int> map;
  const int INSERTIONS = 1000;
  for (int i = 0; i < INSERTIONS; ++i) {
    map.insert(DICT[i], i);
  }

  for (int i = 0; i < INSERTIONS; ++i) {
    assert(*map.find(DICT[i]) == i);
  }
}

int main() {
  test_basic();
  test_save_load();
  test_flat_map();
  test_flat_map();
  return 0;
}
