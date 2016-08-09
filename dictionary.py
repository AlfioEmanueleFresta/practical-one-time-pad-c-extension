def list_of_words(of_length=None):
    """
    Returns a list of words from a dictionary.
    :param of_length: Length. If not specified, all words will be returned.
    :return: A list of lowercase words.
    """

    if of_length:
        dictionary_file = "utils/dictionary/words%d.txt" % of_length
    else:
        dictionary_file = "utils/dictionary/words.txt"

    try:
        dictionary = open(dictionary_file, 'r')
        words = [x.rstrip('\n') for x in dictionary.readlines()]
        dictionary.close()

    except IOError:
        raise Exception("Unable to find or read dictionary (%s)." % dictionary_file)

    return words