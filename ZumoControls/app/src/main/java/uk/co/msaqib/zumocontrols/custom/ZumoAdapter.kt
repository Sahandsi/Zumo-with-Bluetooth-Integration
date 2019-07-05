package uk.co.msaqib.zumocontrols.custom

import android.content.Context
import android.content.res.Resources
import android.support.v4.content.ContextCompat
import android.support.v7.widget.CardView
import android.support.v7.widget.RecyclerView
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.LinearLayout
import android.widget.TextView
import com.airbnb.lottie.LottieAnimationView
import uk.co.msaqib.zumocontrols.R
import uk.co.msaqib.zumocontrols.custom.ZumoAdapter.ViewHolder

import java.util.ArrayList

class ZumoAdapter(private var zumoItems: MutableList<ZumoItem>, var context: Context) : RecyclerView.Adapter<ZumoAdapter.ViewHolder>() {
    override fun onCreateViewHolder(viewGroup: ViewGroup, i: Int): ViewHolder {
        val view = LayoutInflater.from(viewGroup.context)
            .inflate(R.layout.zumo_item, viewGroup, false)
        return ViewHolder(view)
    }

    override fun onBindViewHolder(viewHolder: ViewHolder, i: Int) {
        val (name, life) = zumoItems[i]
        viewHolder.name.text = name
        viewHolder.life.text = life

        if(life.toInt() <= 0){
            viewHolder.card_view.setCardBackgroundColor(context.resources.getColor(R.color.lightRed))
            updateAnim(viewHolder.av_life, "death-heart.json")
        }else if(life.toInt() in 1..60){
            viewHolder.card_view.setCardBackgroundColor(context.resources.getColor(R.color.sunflower))
            updateAnim(viewHolder.av_life, "heart.json")
        }else{
            viewHolder.card_view.setCardBackgroundColor(context.resources.getColor(R.color.colorWhite))
            updateAnim(viewHolder.av_life, "heart.json")
        }
    }

    fun updateAnim(view: LottieAnimationView, file: String){
        view.setAnimation(file)
        view.playAnimation()
        view.loop(true)
    }


    fun updateData(items: MutableList<ZumoItem>) {
        zumoItems = items
        notifyDataSetChanged()
    }

    override fun getItemCount(): Int {
        return zumoItems.size
    }

    class ViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {

        internal var name: TextView
        internal var life: TextView
        internal var card_view: CardView
        internal var parent: LinearLayout
        internal var av_life: LottieAnimationView

        init {
            name = itemView.findViewById(R.id.name)
            life = itemView.findViewById(R.id.life)
            card_view = itemView.findViewById(R.id.card_view)
            parent = itemView.findViewById(R.id.parent)
            av_life = itemView.findViewById(R.id.life_anim)
        }
    }
}
