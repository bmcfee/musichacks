# -*- coding: utf-8 -*-
# <nbformat>3.0</nbformat>

# <codecell>

import nltk
import cPickle as pickle

arpabet = nltk.corpus.cmudict.dict()

English = None
food_mapping = None

def init(CFG):
    global English
    global food_mapping

    with open(CFG['server']['rhymer_data'], 'r') as f:
        English, food_mapping = pickle.load(f)


def syllabify(language, word) :
	'''Syllabifies the word, given a language configuration loaded with loadLanguage.
	   word is either a string of phonemes from the CMU pronouncing dictionary set
	   (with optional stress numbers after vowels), or a Python list of phonemes,
	   e.g. "B AE1 T" or ["B", "AE1", "T"]'''
	   
	if type(word) == str :
		word = word.split()
		
	syllables = [] # This is the returned data structure.

	internuclei = [] # This maintains a list of phonemes between nuclei.
	
	for phoneme in word :
	
		phoneme = phoneme.strip()
		if phoneme == "" :
			continue
		stress = None
		if phoneme[-1].isdigit() :
			stress = int(phoneme[-1])
			phoneme = phoneme[0:-1]
		
		if phoneme in language["vowels"] :
			# Split the consonants seen since the last nucleus into coda and onset.
			
			coda = None
			onset = None
			
			# If there is a period in the input, split there.
			if "." in internuclei :
				period = internuclei.index(".")
				coda = internuclei[:period]
				onset = internuclei[period+1:]
			
			else :
				# Make the largest onset we can. The 'split' variable marks the break point.
				for split in range(0, len(internuclei)+1) :
					coda = internuclei[:split]
					onset = internuclei[split:]
					
					# If we are looking at a valid onset, or if we're at the start of the word
					# (in which case an invalid onset is better than a coda that doesn't follow
					# a nucleus), or if we've gone through all of the onsets and we didn't find
					# any that are valid, then split the nonvowels we've seen at this location.
					if " ".join(onset) in language["onsets"] \
					   or len(syllables) == 0 \
					   or len(onset) == 0 :
					   break
			   
			# Tack the coda onto the coda of the last syllable. Can't do it if this
			# is the first syllable.
			if len(syllables) > 0 :
				syllables[-1][3].extend(coda)
			
			# Make a new syllable out of the onset and nucleus.
			syllables.append( (stress, onset, [phoneme], []) )
				
			# At this point we've processed the internuclei list.
			internuclei = []

		elif not phoneme in language["consonants"] and phoneme != "." :
			raise ValueError, "Invalid phoneme: " + phoneme
			
		else : # a consonant
			internuclei.append(phoneme)
	
	# Done looping through phonemes. We may have consonants left at the end.
	# We may have even not found a nucleus.
	if len(internuclei) > 0 :
		if len(syllables) == 0 :
			syllables.append( (None, internuclei, [], []) )
		else :
			syllables[-1][3].extend(internuclei)

	return syllables

def analyze_rhyme(phonemes):
    ''' Returns the stress pattern, and the rhyme scheme of the last syllable '''
    syllables = syllabify(English, ' '.join(phonemes))
    
    stresses = [int(x[0]>0) for x in syllables]
    tail     = filter(lambda x: len(x) > 0, syllables[-1][1:])
    
    # Crunch the tail?
    # Only if the last phoneme is a vowel and the first is a consonant
    if ' '.join(tail[0]) in English['onsets'] and ''.join(tail[-1]) not in English['vowels']:
        tail = tail[1:]
    
    _tail = []
    for t in tail:
        _tail.extend(t)
    return stresses, _tail

def rhyme_searcher(query):
    query = query.lower()
    
    results = set()
    if query in arpabet:
        for p in arpabet[query]:
            stresses, tail = analyze_rhyme(p)
            key_str = ''.join(map(str, stresses))
            key_end = '_'.join(tail)
    
            if key_str in food_mapping:
                if key_end in food_mapping[key_str]:
                    results.update(food_mapping[key_str][key_end])
                    
    return results

def string_query(query):
    
    # Tokenize the query
    tokens = nltk.tokenize.word_tokenize(query)
    
    # Part-of-speech tag
    tags = nltk.pos_tag(tokens)
    
    results = set()
    
    for word in filter(lambda x: x[-1][0] == 'N', tags):
        
        replacements = rhyme_searcher(word[0])
        
        if len(replacements) == 0:
            continue
        
        for r in replacements:
            results.add(query.replace(word[0], r))
            
    # Return everything except the original query
    return sorted(list(results - set([query])))

def search(query, k=30):
    return string_query(query)[:k]
