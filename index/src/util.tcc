template<typename T>
void binaryInsert(std::vector<T>& array, T element) {
    if(array.empty()) {
        array.push_back(element);
        return;
    }

    int low = 0;
    int high = array.size()-1;
    int mid = 0;

    while(true) {
        mid = low + (high - low) / 2;

        if(element.docID < array[mid].docID) {
            high = mid - 1;
            if(low > high) {
                break;
            }
        }
        else if(element.docID > array[mid].docID) {
            low = mid + 1;
            if(low > high) {
                mid += 1;
                break;
            }
        }
        else 
            break;
    }

    array.insert(array.begin() + mid, element);
}