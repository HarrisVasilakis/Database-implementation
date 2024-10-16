Χαράλαμπος Βασιλάκης sdi1500015
Φωκίωνας Κωσταντάρας sdi1500079

Για να τρέξω το πρόγραμμα χρειάζομαι τα εξης αρχεία:
gcc main.c HT.c BF_64.a
Έχω φτιάξει και μια makefile που τρέχει με ./prog
Oι struct eyrethrio kai SecIndex και blockstruct χρησιμοποιούνται για να αποθηκεύω σε αυτές όλα όσο χρειάζομαι για να βάλω σε ένα block της μνήμης έτσι ώστε να με τη readblock kai memcpy να μπορώ να παίρνω ότι χρειάζομαι αμέσως με μία memcpy. Και αντίστοιχα να γράφω το block πιο εύκολα στη μνήμη.
Το  block 0 θα περιέχει πάντα το eyrethrio που αποτελείται απο το HT_info και έναν πίνακα ints ο οποίος είναι το hash_table μου(τα buckets).Ύστερα για κάθε άλλο block αποθηκεύω τον αριθμό του σε αυτόν τον πίνακα.
Για τη insertentry παίρνω περιπτώσεις.Αν υπάρχει block στη θέση του πίνακα που έχω βρει απ το hash_function(universal hashing) τοτε to κάνω read αλλιώς κάνω allocate.Αν το block έχει θέσεις ελέυθερες,που αυτό καθορίζεται απ την μεταβλητη recordcounter, τότε το βάζω στη θέση recordcounter. Αλλιώς αν δε δείχνει σε άλλο block κάνω allocate, αλλώς με ένα while ψάχνω να βρώ το τελευταίο block.
Στην deleteentry ψάχνω το id και αν το βρώ του αναθέτω τη τιμή -1 υπονοώντας ότι δεν υπάρχει πλέον.Στην getallentries παρόμοια.
Για το secondaryindex δουλεύω παρόμοια αλλά στην create προσπελαύνω ολόκληρη τη primaryindex και κάνω secondaryinsertentry ότι βρώ.
The maxrecords is fixed to 6.

